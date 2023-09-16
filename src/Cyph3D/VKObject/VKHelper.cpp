#include "VKHelper.h"

#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"
#include "Cyph3D/VKObject/VKContext.h"

size_t VKHelper::alignUp(size_t size, size_t alignment)
{
	return ((size + alignment - 1) / alignment) * alignment;
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
