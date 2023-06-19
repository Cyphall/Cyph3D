#include "VKHelper.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

size_t VKHelper::alignUp(size_t size, size_t alignment)
{
	return ((size + alignment - 1) / alignment) * alignment;
}

void VKHelper::buildRaygenShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer)
{
	uint32_t handleSizeAligned = VKHelper::alignUp(32, context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	vk::DeviceSize stride = VKHelper::alignUp(handleSizeAligned, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	vk::DeviceSize size = stride * 1;
	
	destBuffer->resizeSmart(size);
	
	std::memcpy(destBuffer->getHostPointer(), pipeline->getRaygenGroupHandle().data(), pipeline->getRaygenGroupHandle().size());
}

void VKHelper::buildMissShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer)
{
	uint32_t handleSizeAligned = VKHelper::alignUp(32, context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	vk::DeviceSize stride = VKHelper::alignUp(handleSizeAligned, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	vk::DeviceSize size = stride * pipeline->getMissGroupCount();
	
	destBuffer->resizeSmart(size);
	
	std::byte* destBufferPtr = destBuffer->getHostPointer();
	for (uint32_t i = 0; i < pipeline->getMissGroupCount(); i++)
	{
		const std::array<std::byte, 32>& handle = pipeline->getMissGroupHandle(i);
		std::memcpy(destBufferPtr, handle.data(), handle.size());
		destBufferPtr += stride;
	}
}

void VKHelper::buildHitShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer)
{
	uint32_t handleSizeAligned = VKHelper::alignUp(32, context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	vk::DeviceSize stride = VKHelper::alignUp(handleSizeAligned, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	vk::DeviceSize size = stride * pipeline->getHitGroupCount();
	
	destBuffer->resizeSmart(size);
	
	std::byte* destBufferPtr = destBuffer->getHostPointer();
	for (uint32_t i = 0; i < pipeline->getMissGroupCount(); i++)
	{
		for (uint32_t j = 0; j < pipeline->getHitGroupCount(i); j++)
		{
			const std::array<std::byte, 32>& handle = pipeline->getHitGroupHandle(i, j);
			std::memcpy(destBufferPtr, handle.data(), handle.size());
			destBufferPtr += stride;
		}
	}
}

vk::ImageAspectFlags VKHelper::getAspect(vk::Format format)
{
	switch (format)
	{
		case vk::Format::eD16Unorm:
		case vk::Format::eX8D24UnormPack32:
		case vk::Format::eD32Sfloat:
			return vk::ImageAspectFlagBits::eDepth;
		case vk::Format::eS8Uint:
			return vk::ImageAspectFlagBits::eStencil;
		case vk::Format::eD16UnormS8Uint:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32SfloatS8Uint:
			return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		default:
			return vk::ImageAspectFlagBits::eColor;
	}
}

void VKHelper::assertImageViewHasUniqueLayout(const VKPtr<VKImageView>& imageView)
{
#if defined(_DEBUG)
	// make sure all referenced layers and levels have the same layout
	const VKPtr<VKImage>& image = imageView->getInfo().getImage();
	vk::ImageLayout layout = image->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	for (uint32_t layer = imageView->getFirstReferencedLayer(); layer <= imageView->getLastReferencedLayer(); layer++)
	{
		for (uint32_t level = imageView->getFirstReferencedLevel(); level <= imageView->getLastReferencedLevel(); level++)
		{
			if (image->getLayout(layer, level) != layout)
			{
				throw;
			}
		}
	}
#endif
}
