#include "VKHelper.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

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

void VKHelper::assertImageViewHasUniqueState(const std::shared_ptr<VKImage>& image, glm::uvec2 layerRange, glm::uvec2 levelRange)
{
#if defined(_DEBUG)
	// make sure all referenced layers and levels have the same layout
	const VKImage::State& state = image->getState(layerRange.x, levelRange.x);
	for (uint32_t layer = layerRange.x; layer <= layerRange.y; layer++)
	{
		for (uint32_t level = levelRange.x; level <= levelRange.y; level++)
		{
			if (image->getState(layer, level) != state)
			{
				throw;
			}
		}
	}
#endif
}
