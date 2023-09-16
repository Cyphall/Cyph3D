#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKPipelineLayout : public VKObject
{
public:
	static VKPtr<VKPipelineLayout> create(VKContext& context, const VKPipelineLayoutInfo& info);
	
	~VKPipelineLayout() override;
	
	const VKPipelineLayoutInfo& getInfo() const;
	
	const vk::PipelineLayout& getHandle();

protected:
	VKPipelineLayout(VKContext& context, const VKPipelineLayoutInfo& info);
	
	vk::PipelineLayout _pipelineLayout;
	
	VKPipelineLayoutInfo _info;
};