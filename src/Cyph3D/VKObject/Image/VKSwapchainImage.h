#pragma once

#include "Cyph3D/VKObject/Image/VKImage.h"

class VKSwapchain;

class VKSwapchainImage : public VKImage
{
public:
	~VKSwapchainImage() override;
	
	VKSwapchain& getSwapchain() const;
	uint32_t getIndex() const;
	
private:
	friend class VKSwapchain;
	
	VKSwapchainImage(VKContext& context, vk::Image handle, vk::Format format, const glm::uvec2& size, VKSwapchain& swapchain, uint32_t index);
	
	VKSwapchain& _swapchain;
	uint32_t _index;
};