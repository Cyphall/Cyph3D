#include "VKSwapchain.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Image/VKSwapchainImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Semaphore/VKSemaphore.h"

#include <GLFW/glfw3.h>
#include <iostream>

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

static SwapChainSupportDetails querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	SwapChainSupportDetails details;
	
	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
	details.formats = device.getSurfaceFormatsKHR(surface);
	details.presentModes = device.getSurfacePresentModesKHR(surface);
	
	return details;
}

std::unique_ptr<VKSwapchain> VKSwapchain::create(VKContext& context, vk::SurfaceKHR surface, VKSwapchain* oldSwapchain)
{
	return std::unique_ptr<VKSwapchain>(new VKSwapchain(context, surface, oldSwapchain));
}

VKSwapchain::VKSwapchain(VKContext& context, vk::SurfaceKHR surface, VKSwapchain* oldSwapchain):
	VKObject(context)
{
	createSwapchain(surface, oldSwapchain);
	createSemaphores();
}

VKSwapchain::~VKSwapchain()
{
	_context.getDevice().destroySwapchainKHR(_swapchain);
}

VKSwapchain::NextImageInfo VKSwapchain::retrieveNextImage()
{
	const VKPtr<VKSemaphore>& semaphore = _semaphores[_nextIndex];
	_nextIndex = (_nextIndex + 1) % _swapchainImages.size();
	
	auto [result, imageIndex] = _context.getDevice().acquireNextImageKHR(_swapchain, UINT64_MAX, semaphore->getHandle(), VK_NULL_HANDLE);
	if (result == vk::Result::eSuboptimalKHR)
	{
		std::cout << "WARNING: Suboptimal swapchain." << std::endl;
	}
	
	return {
		.image = _swapchainImages[imageIndex],
		.imageView = _swapchainImageViews[imageIndex],
		.imageAvailableSemaphore = semaphore
	};
}

const vk::SwapchainKHR& VKSwapchain::getHandle()
{
	return _swapchain;
}

vk::Format VKSwapchain::getFormat() const
{
	return _swapchainImages.front()->getFormat();
}

const glm::uvec2& VKSwapchain::getSize() const
{
	return _swapchainImages.front()->getSize(0);
}

size_t VKSwapchain::getImageCount() const
{
	return _swapchainImages.size();
}

uint64_t VKSwapchain::getNextPresentId() const
{
	return _nextPresentId;
}

void VKSwapchain::createSwapchain(vk::SurfaceKHR surface, VKSwapchain* oldSwapchain)
{
	SwapChainSupportDetails swapchainSupport = querySwapchainSupport(_context.getPhysicalDevice(), surface);
	
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = 3;
	createInfo.imageFormat = vk::Format::eB8G8R8A8Unorm;
	createInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	createInfo.imageExtent = swapchainSupport.capabilities.currentExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	createInfo.queueFamilyIndexCount = 0; // Optional
	createInfo.pQueueFamilyIndices = nullptr; // Optional
	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = vk::PresentModeKHR::eFifo;
	createInfo.clipped = true;
	createInfo.oldSwapchain = oldSwapchain != nullptr ? oldSwapchain->getHandle() : VK_NULL_HANDLE;
	
	_swapchain = _context.getDevice().createSwapchainKHR(createInfo);
	
	std::vector<vk::Image> swapchainImages = _context.getDevice().getSwapchainImagesKHR(_swapchain);
	_swapchainImages.reserve(swapchainImages.size());
	_swapchainImageViews.reserve(swapchainImages.size());
	for (int i = 0; i < swapchainImages.size(); i++)
	{
		_swapchainImages.push_back(VKPtr<VKSwapchainImage>(new VKSwapchainImage(
			_context,
			swapchainImages[i],
			createInfo.imageFormat,
			glm::uvec2(createInfo.imageExtent.width, createInfo.imageExtent.height),
			*this,
			i
		)));
		
		_swapchainImageViews.push_back(VKImageView::create(
			_context,
			_swapchainImages.back(),
			vk::ImageViewType::e2D));
	}
}

void VKSwapchain::createSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	
	_semaphores.reserve(_swapchainImages.size());
	for (int i = 0; i < _swapchainImages.size(); i++)
	{
		_semaphores.push_back(VKSemaphore::create(_context, semaphoreCreateInfo));
	}
}

void VKSwapchain::onPresent()
{
	_nextPresentId++;
}