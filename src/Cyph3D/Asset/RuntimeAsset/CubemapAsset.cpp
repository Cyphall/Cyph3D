#include "CubemapAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <format>
#include <magic_enum.hpp>

CubemapAsset::CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_bindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
	_manager.addThreadPoolTask(&CubemapAsset::load_async, this);
}

CubemapAsset::~CubemapAsset()
{
	_manager.getBindlessTextureManager().releaseIndex(_bindlessIndex);
}

const uint32_t& CubemapAsset::getBindlessIndex() const
{
	checkLoaded();
	return _bindlessIndex;
}

void CubemapAsset::load_async(AssetManagerWorkerData& workerData)
{
	std::reference_wrapper<std::string> paths[6] = {
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath,
	};

	vk::Format format;
	glm::uvec2 size;
	std::array<std::vector<std::vector<std::byte>>, 6> faces;
	if (!_signature.equirectangularPath.empty())
	{
		EquirectangularSkyboxData equirectangularSkyboxData = _manager.getAssetProcessor().readEquirectangularSkyboxData(workerData, _signature.equirectangularPath);
		format = equirectangularSkyboxData.format;
		size = equirectangularSkyboxData.size;
		faces = std::move(equirectangularSkyboxData.faces);
	}
	else
	{
		uint32_t levels;
		for (uint32_t i = 0; i < 6; i++)
		{
			ImageData imageData = _manager.getAssetProcessor().readImageData(workerData, paths[i].get(), _signature.type);
			faces[i] = std::move(imageData.levels);

			if (i == 0)
			{
				format = imageData.format;
				size = imageData.size;
				levels = imageData.levels.size();
			}
			else
			{
				if (format != imageData.format)
					throw std::runtime_error("All 6 faces of a cubemap must have the same format.");
				if (size != imageData.size)
					throw std::runtime_error("All 6 faces of a cubemap must have the same size.");
				if (levels != imageData.levels.size())
					throw std::runtime_error("All 6 faces of a cubemap must have the same level count.");
			}
		}
	}

	if (!_signature.equirectangularPath.empty())
	{
		Logger::info("Uploading cubemap [equirectangular: {}]...", _signature.equirectangularPath);
	}
	else
	{
		Logger::info(
			"Uploading cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})]...",
			_signature.xposPath,
			_signature.xnegPath,
			_signature.yposPath,
			_signature.ynegPath,
			_signature.zposPath,
			_signature.znegPath,
			magic_enum::enum_name(_signature.type)
		);
	}

	// create cubemap
	VKImageInfo imageInfo(
		format,
		size,
		6,
		faces[0].size(),
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.enableCubeCompatibility();

	if (!_signature.equirectangularPath.empty())
	{
		imageInfo.setName(_signature.equirectangularPath);
	}
	else
	{
		imageInfo.setName(std::format(
			"{}|{}|{}|{}|{}|{}",
			_signature.xposPath,
			_signature.xnegPath,
			_signature.yposPath,
			_signature.ynegPath,
			_signature.zposPath,
			_signature.znegPath
		));
	}

	_image = VKImage::create(Engine::getVKContext(), imageInfo);

	// create staging buffer
	VKBufferInfo bufferInfo(_image->getLayerByteSize() * 6, vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);

	// copy face data to staging buffer
	std::byte* ptr = stagingBuffer->getHostPointer();
	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t level = 0; level < faces[face].size(); level++)
		{
			if (_image->getLevelByteSize(level) != faces[face][level].size())
				throw;

			std::copy_n(faces[face][level].data(), faces[face][level].size(), ptr);
			ptr += faces[face][level].size();
		}
	}

	// upload staging buffer to texture
	workerData.transferCommandBuffer->begin();

	workerData.transferCommandBuffer->bufferMemoryBarrier(
		stagingBuffer,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferRead
	);

	workerData.transferCommandBuffer->imageMemoryBarrier(
		_image,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal
	);

	vk::DeviceSize bufferOffset = 0;
	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t i = 0; i < _image->getInfo().getLevels(); i++)
		{
			workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, face, i);
			bufferOffset += _image->getLevelByteSize(i);
		}
	}

	workerData.transferCommandBuffer->releaseImageOwnership(
		_image,
		Engine::getVKContext().getMainQueue(),
		vk::ImageLayout::eReadOnlyOptimal
	);

	workerData.transferCommandBuffer->end();

	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});

	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();

	workerData.graphicsCommandBuffer->begin();

	vk::PipelineStageFlags2 nextUsageStages = vk::PipelineStageFlagBits2::eFragmentShader;
	if (Engine::getVKContext().isRayTracingSupported())
	{
		nextUsageStages |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
	}

	workerData.graphicsCommandBuffer->acquireImageOwnership(
		_image,
		Engine::getVKContext().getMainQueue(),
		nextUsageStages,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	workerData.graphicsCommandBuffer->end();

	Engine::getVKContext().getMainQueue().submit(workerData.graphicsCommandBuffer, {}, {});

	workerData.graphicsCommandBuffer->waitExecution();
	workerData.graphicsCommandBuffer->reset();

	// set texture to bindless descriptor set
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _image, _manager.getCubemapSampler());

	_loaded = true;
	if (!_signature.equirectangularPath.empty())
	{
		Logger::info("Cubemap [equirectangular: {}] uploaded succesfully", _signature.equirectangularPath);
	}
	else
	{
		Logger::info(
			"Cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})] uploaded succesfully",
			_signature.xposPath,
			_signature.xnegPath,
			_signature.yposPath,
			_signature.ynegPath,
			_signature.zposPath,
			_signature.znegPath,
			magic_enum::enum_name(_signature.type)
		);
	}

	_changed();
}