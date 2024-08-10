#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKSemaphore : public VKObject
{
public:
	static std::shared_ptr<VKSemaphore> create(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo);

	~VKSemaphore() override;

	const vk::Semaphore& getHandle();

private:
	VKSemaphore(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo);

	vk::Semaphore _semaphore;
};