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
{}

const VKPtr<VKImageView>& CubemapAsset::getImageView() const
{
	checkLoaded();
	return _imageView;
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
	std::array<ImageData, 6> imageDataList;
	for (uint32_t i = 0; i < 6; i++)
	{
		imageDataList[i] = _manager.readImageData(paths[i].get(), ImageType::ColorSrgb);
		
		if (i == 0)
		{
			format = imageDataList[i].format;
			size = imageDataList[i].size;
		}
		else
		{
			if (format != imageDataList[i].format)
				throw std::runtime_error("All 6 faces of a cubemap must have the same format.");
			if (size != imageDataList[i].size)
				throw std::runtime_error("All 6 faces of a cubemap must have the same size.");
		}
	}
	
	_image = VKImage::create(
		Engine::getVKContext(),
		format,
		size,
		6,
		VKImage::calcMaxMipLevels(size),
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
		std::copy(imageDataList[face].data.begin(), imageDataList[face].data.end(), ptr);
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