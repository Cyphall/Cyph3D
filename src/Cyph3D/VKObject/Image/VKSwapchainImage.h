#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKSwapchain;
class VKImage;

class VKSwapchainImage : public VKObject
{
public:
	static VKPtr<VKSwapchainImage> create(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);
	
	~VKSwapchainImage() override;
	
	VKSwapchain& getSwapchain() const;
	const VKPtr<VKImage>& getImage();
	const uint32_t& getIndex() const;
	
private:
	VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);
	
	VKSwapchain& _swapchain;
	VKPtr<VKImage> _image;
	uint32_t _index;
};