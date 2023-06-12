#include "TextureAsset.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <format>
#include <magic_enum.hpp>

TextureAsset::TextureAsset(AssetManager& manager, const TextureAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading texture {} with type {}", _signature.path, magic_enum::enum_name(_signature.type)));
	_manager.addMainThreadTask(&TextureAsset::load_step1_mt, this);
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

bool TextureAsset::load_step1_mt()
{
	ImageData imageData = _manager.getAssetProcessor().readImageData(_signature.path, _signature.type);
	
	VKImageInfo imageInfo(
		imageData.format,
		imageData.size,
		1,
		imageData.levels.size(),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_image = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(
		Engine::getVKContext(),
		_image->getLayerByteSize(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
	
	std::byte* ptr = stagingBuffer->map();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		vk::DeviceSize byteSize = _image->getLevelByteSize(i);
		std::memcpy(ptr, imageData.levels[i].data.data(), byteSize);
		ptr += byteSize;
	}
	stagingBuffer->unmap();
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			vk::DeviceSize bufferOffset = 0;
			for (uint32_t i = 0; i < _image->getInfo().getLevels(); i++)
			{
				commandBuffer->imageMemoryBarrier(
					_image,
					0,
					i,
					vk::PipelineStageFlagBits2::eNone,
					vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eCopy,
					vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eTransferDstOptimal);
				
				commandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, 0, i);
				bufferOffset += _image->getLevelByteSize(i);
				
				commandBuffer->imageMemoryBarrier(
					_image,
					0,
					i,
					vk::PipelineStageFlagBits2::eCopy,
					vk::AccessFlagBits2::eTransferWrite,
					vk::PipelineStageFlagBits2::eFragmentShader,
					vk::AccessFlagBits2::eShaderSampledRead,
					vk::ImageLayout::eReadOnlyOptimal);
			}
		});
	
	VKImageViewInfo imageViewInfo(
		_image,
		vk::ImageViewType::e2D);
	
	_imageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	
	_bindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _imageView, _manager.getTextureSampler());

	_loaded = true;
	Logger::info(std::format("Texture {} with type {} loaded", _signature.path, magic_enum::enum_name(_signature.type)));
	
	return true;
}