#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class VKSwapchain;
class VKImage;

class VKSwapchainImage : public VKObject
{
public:
	static VKPtr<VKSwapchainImage> create(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);
	
	~VKSwapchainImage() override;
	
	VKSwapchain& getSwapchain() const;
	const VKPtr<VKImage>& getImage();
	uint32_t getIndex() const;
	
private:
	VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);
	
	VKSwapchain& _swapchain;
	VKPtr<VKImage> _image;
	uint32_t _index;
};