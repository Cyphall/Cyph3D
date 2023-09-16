#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKSwapchainImage;
class VKImageView;
class VKSemaphore;

class VKSwapchain : public VKObject
{
public:
	struct NextImageInfo
	{
		const VKPtr<VKSwapchainImage>& image;
		const VKPtr<VKImageView>& imageView;
		const VKPtr<VKSemaphore>& imageAvailableSemaphore;
	};
	
	static std::unique_ptr<VKSwapchain> create(VKContext& context, vk::SurfaceKHR surface, VKSwapchain* oldSwapchain = nullptr);
	
	~VKSwapchain() override;
	
	NextImageInfo retrieveNextImage();
	
	const vk::SwapchainKHR& getHandle();
	
	vk::Format getFormat() const;
	const glm::uvec2& getSize() const;
	
	size_t getImageCount() const;
	
private:
	friend class VKQueue;
	
	explicit VKSwapchain(VKContext& context, vk::SurfaceKHR surface, VKSwapchain* oldSwapchain);
	
	void createSwapchain(vk::SurfaceKHR surface, VKSwapchain* oldSwapchain);
	void createSemaphores();
	
	vk::SwapchainKHR _swapchain;
	
	std::vector<VKPtr<VKSwapchainImage>> _swapchainImages;
	std::vector<VKPtr<VKImageView>> _swapchainImageViews;
	
	std::vector<VKPtr<VKSemaphore>> _semaphores;
	size_t _nextIndex = 0;
};