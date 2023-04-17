#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"

#include <vulkan/vulkan.hpp>

class VKGraphicsPipelineInfo;

class VKGraphicsPipeline : public VKPipeline
{
public:
	static VKPtr<VKGraphicsPipeline> create(VKContext& context, VKGraphicsPipelineInfo& info);
	static VKDynamic<VKGraphicsPipeline> createDynamic(VKContext& context, VKGraphicsPipelineInfo& info);
	
	~VKGraphicsPipeline() override;
	
	vk::PipelineBindPoint getPipelineType() override;
	
private:
	VKGraphicsPipeline(VKContext& context, VKGraphicsPipelineInfo& info);
	
	void createPipeline(VKGraphicsPipelineInfo& info);
};