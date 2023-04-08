#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKPipelineLayoutInfo;
class VKDescriptorSetLayout;

class VKPipelineLayout : public VKObject
{
public:
	static VKPtr<VKPipelineLayout> create(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo);
	static VKDynamic<VKPipelineLayout> createDynamic(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo);
	
	~VKPipelineLayout() override;
	
	const vk::PipelineLayout& getHandle();
	
	const VKPtr<VKDescriptorSetLayout>& getDescriptorSetLayout(uint32_t setIndex);

protected:
	VKPipelineLayout(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo);
	
	vk::PipelineLayout _pipelineLayout;
	std::vector<VKPtr<VKDescriptorSetLayout>> _descriptorSetsLayouts;
};