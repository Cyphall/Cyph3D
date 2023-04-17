#include "VKPipeline.h"

VKPipeline::VKPipeline(VKContext& context, const VKPtr<VKPipelineLayout>& pipelineLayout):
	VKObject(context), _pipelineLayout(pipelineLayout)
{
	
}

const vk::Pipeline& VKPipeline::getHandle()
{
	return _pipeline;
}

const VKPtr<VKPipelineLayout>& VKPipeline::getPipelineLayout() const
{
	return _pipelineLayout;
}