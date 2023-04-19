#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>

class VKDescriptorSetLayout : public VKObject
{
public:
	static VKPtr<VKDescriptorSetLayout> create(VKContext& context, const VKDescriptorSetLayoutInfo& info);
	static VKDynamic<VKDescriptorSetLayout> createDynamic(VKContext& context, const VKDescriptorSetLayoutInfo& info);
	
	~VKDescriptorSetLayout() override;
	
	const VKDescriptorSetLayoutInfo& getInfo() const;
	
	const vk::DescriptorSetLayout& getHandle();

private:
	VKDescriptorSetLayout(VKContext& context, const VKDescriptorSetLayoutInfo& info);
	
	VKDescriptorSetLayoutInfo _info;
	
	vk::DescriptorSetLayout _handle;
};