#pragma once

#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <vulkan/vulkan.hpp>

class VKGraphicsPipeline : public VKPipeline
{
public:
	static VKPtr<VKGraphicsPipeline> create(VKContext& context, VKGraphicsPipelineInfo& info);

	~VKGraphicsPipeline() override;

	const VKGraphicsPipelineInfo& getInfo() const;

	vk::PipelineBindPoint getPipelineType() const override;
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKGraphicsPipeline(VKContext& context, VKGraphicsPipelineInfo& info);

	VKGraphicsPipelineInfo _info;
};