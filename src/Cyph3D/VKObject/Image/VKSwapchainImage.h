#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKSwapchain;
class VKImage;

class VKSwapchainImage : public VKObject
{
public:
	static std::shared_ptr<VKSwapchainImage> create(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);

	~VKSwapchainImage() override;

	VKSwapchain& getSwapchain() const;
	const std::shared_ptr<VKImage>& getImage();
	const uint32_t& getIndex() const;

private:
	VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);

	VKSwapchain& _swapchain;
	std::shared_ptr<VKImage> _image;
	uint32_t _index;
};