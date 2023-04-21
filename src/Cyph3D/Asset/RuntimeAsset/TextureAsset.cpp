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
{}

const VKPtr<VKImageView>& TextureAsset::getImageView() const
{
	checkLoaded();
	return _imageView;
}

bool TextureAsset::load_step1_mt()
{
	ImageData imageData = _manager.readImageData(_signature.path, _signature.type);
	
	_image = VKImage::create(
		Engine::getVKContext(),
		imageData.format,
		imageData.size,
		1,
		imageData.levels.size(),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
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
			for (uint32_t i = 0; i < _image->getLevels(); i++)
			{
				commandBuffer->imageMemoryBarrier(
					_image,
					vk::PipelineStageFlagBits2::eNone,
					vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eCopy,
					vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eTransferDstOptimal,
					0,
					i);
				
				commandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, 0, i);
				bufferOffset += _image->getLevelByteSize(i);
				
				commandBuffer->imageMemoryBarrier(
					_image,
					vk::PipelineStageFlagBits2::eCopy,
					vk::AccessFlagBits2::eTransferWrite,
					vk::PipelineStageFlagBits2::eFragmentShader,
					vk::AccessFlagBits2::eShaderSampledRead,
					vk::ImageLayout::eReadOnlyOptimal,
					0,
					i);
			}
		});
	
	_imageView = VKImageView::create(
		Engine::getVKContext(),
		_image,
		vk::ImageViewType::e2D);

	_loaded = true;
	Logger::info(std::format("Texture {} with type {} loaded", _signature.path, magic_enum::enum_name(_signature.type)));
	
	return true;
}