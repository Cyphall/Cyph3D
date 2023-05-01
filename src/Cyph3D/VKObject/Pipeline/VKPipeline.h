#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKPipelineLayout;

class VKPipeline : public VKObject
{
public:
	const vk::Pipeline& getHandle();
	
	virtual vk::PipelineBindPoint getPipelineType() const = 0;
	virtual const VKPtr<VKPipelineLayout>& getPipelineLayout() const = 0;

protected:
	explicit VKPipeline(VKContext& context);
	
	vk::Pipeline _pipeline;
};