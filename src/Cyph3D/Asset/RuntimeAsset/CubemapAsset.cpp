#include "CubemapAsset.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

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
	uint32_t levels;
	std::array<ImageData, 6> imageDataList;
	for (uint32_t i = 0; i < 6; i++)
	{
		imageDataList[i] = _manager.getAssetProcessor().readImageData(workerData, paths[i].get(), _signature.type);
		
		if (i == 0)
		{
			format = imageDataList[i].format;
			size = imageDataList[i].size;
			levels = imageDataList[i].levels.size();
		}
		else
		{
			if (format != imageDataList[i].format)
				throw std::runtime_error("All 6 faces of a cubemap must have the same format.");
			if (size != imageDataList[i].size)
				throw std::runtime_error("All 6 faces of a cubemap must have the same size.");
			if (levels != imageDataList[i].levels.size())
				throw std::runtime_error("All 6 faces of a cubemap must have the same level count.");
		}
	}
	
	Logger::info(std::format("Uploading cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})]...",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath,
		magic_enum::enum_name(_signature.type)));
	
	// create cubemap
	VKImageInfo imageInfo(
		format,
		size,
		6,
		levels,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.enableCubeCompatibility();
	
	_image = VKImage::create(Engine::getVKContext(), imageInfo);
	
	// create cubemap view
	VKImageViewInfo imageViewInfo(
		_image,
		vk::ImageViewType::eCube);
	
	_imageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	
	// create staging buffer
	VKBufferInfo bufferInfo(_image->getLayerByteSize(), vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);
	
	for (uint32_t face = 0; face < 6; face++)
	{
		// copy face data to staging buffer
		std::byte* ptr = stagingBuffer->getHostPointer();
		for (uint32_t i = 0; i < imageDataList[face].levels.size(); i++)
		{
			vk::DeviceSize byteSize = _image->getLevelByteSize(i);
			std::memcpy(ptr, imageDataList[face].levels[i].data.data(), byteSize);
			ptr += byteSize;
		}
		
		// upload staging buffer to texture
		workerData.transferCommandBuffer->begin();
		
		vk::DeviceSize bufferOffset = 0;
		for (uint32_t i = 0; i < _image->getInfo().getLevels(); i++)
		{
			workerData.transferCommandBuffer->imageMemoryBarrier(
				_image, face, i,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal);
			
			workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, face, i);
			bufferOffset += _image->getLevelByteSize(i);
			
			workerData.transferCommandBuffer->imageMemoryBarrier(
				_image, face, i,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::ImageLayout::eReadOnlyOptimal);
		}
		
		workerData.transferCommandBuffer->end();
		
		Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});
		
		workerData.transferCommandBuffer->waitExecution();
		workerData.transferCommandBuffer->reset();
	}
	
	// set texture to bindless descriptor set
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _imageView, _manager.getCubemapSampler());
	
	_changed();

	_loaded = true;
	Logger::info(std::format("Cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})] uploaded succesfully",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath,
		magic_enum::enum_name(_signature.type)));
}