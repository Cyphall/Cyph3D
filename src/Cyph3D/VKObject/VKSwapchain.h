#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKSwapchainImage;
class VKSemaphore;

class VKSwapchain : public VKObject
{
public:
	struct NextImageInfo
	{
		const std::shared_ptr<VKSwapchainImage>& image;
		const std::shared_ptr<VKSemaphore>& imageAvailableSemaphore;
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

	std::vector<std::shared_ptr<VKSwapchainImage>> _swapchainImages;

	std::vector<std::shared_ptr<VKSemaphore>> _semaphores;
	size_t _nextIndex = 0;
};