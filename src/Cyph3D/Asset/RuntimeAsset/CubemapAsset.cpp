#include "CubemapAsset.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <format>

CubemapAsset::CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading cubemap (xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {})",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath));
	_manager.addMainThreadTask(&CubemapAsset::load_step1_mt, this);
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

bool CubemapAsset::load_step1_mt()
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
		imageDataList[i] = _manager.readImageData(paths[i].get(), ImageType::ColorSrgb);
		
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
	
	_image = VKImage::create(
		Engine::getVKContext(),
		format,
		size,
		6,
		levels,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		{},
		true);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(
		Engine::getVKContext(),
		_image->getLayerByteSize(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
	
	for (uint32_t face = 0; face < 6; face++)
	{
		std::byte* ptr = stagingBuffer->map();
		for (uint32_t i = 0; i < imageDataList[face].levels.size(); i++)
		{
			vk::DeviceSize byteSize = _image->getLevelByteSize(i);
			std::memcpy(ptr, imageDataList[face].levels[i].data.data(), byteSize);
			ptr += byteSize;
		}
		stagingBuffer->unmap();
		
		Engine::getVKContext().executeImmediate(
			[&](const VKPtr<VKCommandBuffer>& commandBuffer)
			{
				vk::DeviceSize bufferOffset = 0;
				for (uint32_t level = 0; level < _image->getLevels(); level++)
				{
					commandBuffer->imageMemoryBarrier(
						_image,
						vk::PipelineStageFlagBits2::eNone,
						vk::AccessFlagBits2::eNone,
						vk::PipelineStageFlagBits2::eCopy,
						vk::AccessFlagBits2::eTransferWrite,
						vk::ImageLayout::eTransferDstOptimal,
						face,
						level);
					
					commandBuffer->copyBufferToImage(stagingBuffer, bufferOffset, _image, face, level);
					bufferOffset += _image->getLevelByteSize(level);
					
					commandBuffer->imageMemoryBarrier(
						_image,
						vk::PipelineStageFlagBits2::eCopy,
						vk::AccessFlagBits2::eTransferWrite,
						vk::PipelineStageFlagBits2::eFragmentShader,
						vk::AccessFlagBits2::eShaderSampledRead,
						vk::ImageLayout::eReadOnlyOptimal,
						face,
						level);
				}
			});
	}
	
	_imageView = VKImageView::create(
		Engine::getVKContext(),
		_image,
		vk::ImageViewType::eCube);
	
	_bindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
	_manager.getBindlessTextureManager().setTexture(_bindlessIndex, _imageView, _manager.getCubemapSampler());

	_loaded = true;
	Logger::info(std::format("Cubemap (xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {}) loaded",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath));

	return true;
}