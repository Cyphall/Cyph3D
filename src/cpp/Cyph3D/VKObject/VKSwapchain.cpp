#include "VKSwapchain.h"

#include <Cyph3D/VKObject/Image/VKImage.h>
#include <Cyph3D/VKObject/Image/VKSwapchainImage.h>
#include <Cyph3D/VKObject/Semaphore/VKSemaphore.h>
#include <Cyph3D/VKObject/VKContext.h>
#include "Fence/VKFence.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <spdlog/spdlog.h>

namespace
{
struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	SwapChainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
	details.formats = device.getSurfaceFormatsKHR(surface);
	details.presentModes = device.getSurfacePresentModesKHR(surface);

	return details;
}

vk::SurfaceFormatKHR findBestSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
	constexpr std::array<vk::SurfaceFormatKHR, 2> preferredSurfaceFormats = {
		vk::SurfaceFormatKHR{
			.format = vk::Format::eA2B10G10R10UnormPack32,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		},
		vk::SurfaceFormatKHR{
			.format = vk::Format::eB8G8R8A8Unorm,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		}
	};

	std::vector<vk::SurfaceFormatKHR> supportedSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

	for (const vk::SurfaceFormatKHR& preferredSurfaceFormat : preferredSurfaceFormats)
	{
		if (std::ranges::find(supportedSurfaceFormats, preferredSurfaceFormat) != supportedSurfaceFormats.end())
		{
			return preferredSurfaceFormat;
		}
	}

	spdlog::error("Could not find a preferred surface format");

	return supportedSurfaceFormats[0];
}
}

std::unique_ptr<c3d::VKSwapchain> c3d::VKSwapchain::create(VKContext& context, vk::SurfaceKHR surface, glm::uvec2 requestedExtent, VKSwapchain* oldSwapchain)
{
	return std::unique_ptr<VKSwapchain>(new VKSwapchain(context, surface, requestedExtent, oldSwapchain));
}

c3d::VKSwapchain::VKSwapchain(VKContext& context, vk::SurfaceKHR surface, glm::uvec2 requestedExtent, VKSwapchain* oldSwapchain):
	VKObject(context)
{
	createSwapchain(surface, requestedExtent, oldSwapchain);
}

c3d::VKSwapchain::~VKSwapchain()
{
	_context.getDevice().destroySwapchainKHR(_swapchain);
}

const std::shared_ptr<c3d::VKSwapchainImage>& c3d::VKSwapchain::retrieveNextImage(VKFence& fence)
{
	_nextIndex = (_nextIndex + 1) % _swapchainImages.size();

	auto [result, imageIndex] = _context.getDevice().acquireNextImageKHR(_swapchain, UINT64_MAX, VK_NULL_HANDLE, fence.getHandle());
	if (result == vk::Result::eSuboptimalKHR)
	{
		spdlog::warn("Suboptimal swapchain");
	}

	return _swapchainImages[imageIndex];
}

const vk::SwapchainKHR& c3d::VKSwapchain::getHandle()
{
	return _swapchain;
}

vk::Format c3d::VKSwapchain::getFormat() const
{
	return _swapchainImages.front()->getImage()->getInfo().getFormat();
}

const glm::uvec2& c3d::VKSwapchain::getSize() const
{
	return _swapchainImages.front()->getImage()->getSize(0);
}

size_t c3d::VKSwapchain::getImageCount() const
{
	return _swapchainImages.size();
}

void c3d::VKSwapchain::createSwapchain(vk::SurfaceKHR surface, glm::uvec2 requestedExtent, VKSwapchain* oldSwapchain)
{
	SwapChainSupportDetails swapchainSupport = querySwapchainSupport(_context.getPhysicalDevice(), surface);
	vk::SurfaceFormatKHR surfaceFormat = findBestSurfaceFormat(_context.getPhysicalDevice(), surface);

	glm::uvec2 extent = glm::clamp(
		requestedExtent,
		std::bit_cast<glm::uvec2>(swapchainSupport.capabilities.minImageExtent),
		std::bit_cast<glm::uvec2>(swapchainSupport.capabilities.maxImageExtent)
	);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = 3;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = std::bit_cast<vk::Extent2D>(extent);
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = vk::PresentModeKHR::eFifo;
	createInfo.clipped = true;
	createInfo.oldSwapchain = oldSwapchain != nullptr ? oldSwapchain->getHandle() : VK_NULL_HANDLE;

	_swapchain = _context.getDevice().createSwapchainKHR(createInfo);

	std::vector<vk::Image> swapchainImages = _context.getDevice().getSwapchainImagesKHR(_swapchain);
	_swapchainImages.reserve(swapchainImages.size());
	for (int i = 0; i < swapchainImages.size(); i++)
	{
		_swapchainImages.push_back(
			VKSwapchainImage::create(
				_context,
				swapchainImages[i],
				createInfo.imageFormat,
				glm::uvec2(createInfo.imageExtent.width, createInfo.imageExtent.height),
				*this,
				i
			)
		);
	}
}