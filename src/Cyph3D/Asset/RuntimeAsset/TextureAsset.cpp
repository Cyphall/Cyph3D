#include "TextureAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <magic_enum.hpp>
#include <format>

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

void TextureAsset::load_async(AssetManagerWorkerData& workerData)
{
	ImageData imageData = _manager.getAssetProcessor().readImageData(workerData, _signature.path, _signature.type);
	
	Logger::info(std::format("Uploading texture [{} ({})]...", _signature.path, magic_enum::enum_name(_signature.type)));
	
	// create texture
	VKImageInfo imageInfo(
		imageData.format,
		imageData.size,
		1,
		imageData.levels.size(),
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_image = VKImage::create(Engine::getVKContext(), imageInfo);
	
	// create texture view
	VKImageViewInfo imageViewInfo(
		_image,
		vk::ImageViewType::e2D);
	
	_imageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	
	// create staging buffer
	VKBufferInfo bufferInfo(_image->getLayerByteSize(),vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);
	
	// copy texture data to staging buffer
	std::byte* ptr = stagingBuffer->getHostPointer();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		vk::DeviceSize byteSize = _image->getLevelByteSize(i);
		std::memcpy(ptr, imageData.levels[i].data.data(), byteSize);
		ptr += byteSize;
	}
	
	// upload staging buffer to texture
	workerData.transferCommandBuffer->begin();
	
	vk::DeviceSize bufferOffset = 0;
	for (uint32_t i = 0; i < _image->getInfo().getLevels(); i++)
	{
		workerData.transferCommandBuffer->imageMemoryBarrier(
			_image, 0, i,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal);
		
		workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, 0, i);
		bufferOffset += _image->getLevelByteSize(i);
		
		workerData.transferCommandBuffer->imageMemoryBarrier(
			_image, 0, i,
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
	
	// set texture to bindless descriptor set
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _imageView, _manager.getTextureSampler());
	
	_changed();
	
	_loaded = true;
	Logger::info(std::format("Texture [{} ({})] uploaded succesfully", _signature.path, magic_enum::enum_name(_signature.type)));
}