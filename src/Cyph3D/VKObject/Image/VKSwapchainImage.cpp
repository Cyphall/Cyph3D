#include "VKSwapchainImage.h"

#include "Cyph3D/VKObject/VKContext.h"

VKSwapchainImage::VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index):
	VKImage(
		context,
		handle,
		format,
		size,
		1,
		1,
		vk::ImageAspectFlagBits::eColor),
	_swapchain(swapchain),
	_index(index)
{

}

VKSwapchainImage::~VKSwapchainImage()
{

}

VKSwapchain& VKSwapchainImage::getSwapchain() const
{
	return _swapchain;
}

uint32_t VKSwapchainImage::getIndex() const
{
	return _index;
}