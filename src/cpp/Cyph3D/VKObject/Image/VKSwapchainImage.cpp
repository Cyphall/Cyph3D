#include "VKSwapchainImage.h"

#include <Cyph3D/VKObject/Image/VKImage.h>
#include <Cyph3D/VKObject/VKContext.h>

std::shared_ptr<c3d::VKSwapchainImage> c3d::VKSwapchainImage::create(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index)
{
	return std::shared_ptr<VKSwapchainImage>(new VKSwapchainImage(context, handle, format, size, swapchain, index));
}

c3d::VKSwapchainImage::VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index):
	VKObject(context),
	_swapchain(swapchain),
	_index(index)
{
	VKImageInfo info(format, size, 1, 1, {});
	info.setSwapchainImageHandle(handle);
	info.setName(std::format("Swapchain image #{}", index));

	_image = VKImage::create(_context, info);
}

c3d::VKSwapchainImage::~VKSwapchainImage() = default;

c3d::VKSwapchain& c3d::VKSwapchainImage::getSwapchain() const
{
	return _swapchain;
}

const std::shared_ptr<c3d::VKImage>& c3d::VKSwapchainImage::getImage()
{
	return _image;
}

const uint32_t& c3d::VKSwapchainImage::getIndex() const
{
	return _index;
}