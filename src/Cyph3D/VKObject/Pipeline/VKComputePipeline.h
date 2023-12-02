#pragma once

#include "Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <vulkan/vulkan.hpp>

class VKComputePipeline : public VKPipeline
{
public:
	static VKPtr<VKComputePipeline> create(VKContext& context, VKComputePipelineInfo& info);

	~VKComputePipeline() override;

	const VKComputePipelineInfo& getInfo() const;

	vk::PipelineBindPoint getPipelineType() const override;
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKComputePipeline(VKContext& context, VKComputePipelineInfo& info);

	VKComputePipelineInfo _info;
};