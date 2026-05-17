#include "TextureAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

TextureAsset::TextureAsset(AssetManager& manager, const TextureAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_bindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
	_manager.addThreadPoolTask(&TextureAsset::load_async, this);
}

TextureAsset::~TextureAsset()
{
	_manager.getBindlessTextureManager().releaseIndex(_bindlessIndex);
}

const uint32_t& TextureAsset::getBindlessIndex() const
{
	checkLoaded();
	return _bindlessIndex;
}

void TextureAsset::load_async()
{
	ImageData imageData = _manager.getAssetProcessor().readImageData(_signature.path, _signature.type);

	spdlog::info("Uploading texture [{} ({})]...", _signature.path, magic_enum::enum_name(_signature.type));

	// create texture
	VKImageInfo imageInfo(
		imageData.format,
		imageData.size,
		1,
		imageData.levels.size(),
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setName(_signature.path);

	_image = VKImage::create(Engine::getVKContext(), imageInfo);

	// create staging buffer
	VKBufferInfo bufferInfo(_image->getLayerByteSize(), vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	std::shared_ptr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);

	// copy texture data to staging buffer
	std::byte* ptr = stagingBuffer->getHostPointer();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		if (_image->getLevelByteSize(i) != imageData.levels[i].size())
			throw;

		std::copy_n(imageData.levels[i].data(), imageData.levels[i].size(), ptr);
		ptr += imageData.levels[i].size();
	}

	// upload staging buffer to texture
	assetTransferCommandBuffer->begin();

	assetTransferCommandBuffer->bufferMemoryBarrier(
		stagingBuffer,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferRead
	);

	assetTransferCommandBuffer->imageMemoryBarrier(
		_image,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal
	);

	vk::DeviceSize bufferOffset = 0;
	for (uint32_t i = 0; i < _image->getInfo().getLevels(); i++)
	{
		assetTransferCommandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, 0, i);
		bufferOffset += _image->getLevelByteSize(i);
	}

	assetTransferCommandBuffer->releaseImageOwnership(
		_image,
		Engine::getVKContext().getMainQueue(),
		vk::ImageLayout::eReadOnlyOptimal
	);

	assetTransferCommandBuffer->end();

	Engine::getVKContext().getTransferQueue().submit(assetTransferCommandBuffer, {}, {});

	assetTransferCommandBuffer->waitExecution();
	assetTransferCommandBuffer->reset();

	assetGraphicsCommandBuffer->begin();

	vk::PipelineStageFlags2 nextUsageStages = vk::PipelineStageFlagBits2::eFragmentShader;
	if (Engine::getVKContext().isRayTracingSupported())
	{
		nextUsageStages |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
	}

	assetGraphicsCommandBuffer->acquireImageOwnership(
		_image,
		Engine::getVKContext().getTransferQueue(),
		nextUsageStages,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	assetGraphicsCommandBuffer->end();

	Engine::getVKContext().getMainQueue().submit(assetGraphicsCommandBuffer, {}, {});

	assetGraphicsCommandBuffer->waitExecution();
	assetGraphicsCommandBuffer->reset();

	// set texture to bindless descriptor set
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _image, _manager.getTextureSampler());

	_loaded = true;
	spdlog::info("Texture [{} ({})] uploaded succesfully", _signature.path, magic_enum::enum_name(_signature.type));

	_changed();
}