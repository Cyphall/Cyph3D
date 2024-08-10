#pragma once

#include "Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"

#include <vulkan/vulkan.hpp>

class VKComputePipeline : public VKPipeline
{
public:
	static std::shared_ptr<VKComputePipeline> create(VKContext& context, const VKComputePipelineInfo& info);

	~VKComputePipeline() override;

	const VKComputePipelineInfo& getInfo() const;

	vk::PipelineBindPoint getPipelineType() const override;
	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKComputePipeline(VKContext& context, const VKComputePipelineInfo& info);

	VKComputePipelineInfo _info;
};