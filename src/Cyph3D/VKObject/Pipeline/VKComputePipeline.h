#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"

#include <vulkan/vulkan.hpp>

class VKComputePipelineInfo;

class VKComputePipeline : public VKPipeline
{
public:
	static VKPtr<VKComputePipeline> create(VKContext& context, VKComputePipelineInfo& info);
	static VKDynamic<VKComputePipeline> createDynamic(VKContext& context, VKComputePipelineInfo& info);
	
	~VKComputePipeline() override;
	
	vk::PipelineBindPoint getPipelineType() override;

private:
	VKComputePipeline(VKContext& context, VKComputePipelineInfo& info);
	
	void createPipeline(VKComputePipelineInfo& info);
};