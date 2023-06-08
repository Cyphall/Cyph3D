#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKSampler : public VKObject
{
public:
	static VKPtr<VKSampler> create(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo);
	
	~VKSampler() override;
	
	const vk::Sampler& getHandle();

private:
	VKSampler(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo);
	
	vk::Sampler _sampler;
};