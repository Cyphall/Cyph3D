#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKPipelineLayout;

class VKPipeline : public VKObject
{
public:
	const vk::Pipeline& getHandle();
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const;
	
	virtual vk::PipelineBindPoint getPipelineType() = 0;

protected:
	explicit VKPipeline(VKContext& context, const VKPtr<VKPipelineLayout>& pipelineLayout);
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	vk::Pipeline _pipeline;
};