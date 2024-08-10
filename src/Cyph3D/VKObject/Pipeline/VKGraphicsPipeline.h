#pragma once

#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"

#include <vulkan/vulkan.hpp>

class VKGraphicsPipeline : public VKPipeline
{
public:
	static std::shared_ptr<VKGraphicsPipeline> create(VKContext& context, const VKGraphicsPipelineInfo& info);

	~VKGraphicsPipeline() override;

	const VKGraphicsPipelineInfo& getInfo() const;

	vk::PipelineBindPoint getPipelineType() const override;
	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKGraphicsPipeline(VKContext& context, const VKGraphicsPipelineInfo& info);

	VKGraphicsPipelineInfo _info;
};