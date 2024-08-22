#include "VKPipeline.h"

VKPipeline::VKPipeline(VKContext& context):
	VKObject(context)
{
}

const vk::Pipeline& VKPipeline::getHandle()
{
	return _pipeline;
}