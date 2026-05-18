#include "VKPipeline.h"

c3d::VKPipeline::VKPipeline(VKContext& context):
	VKObject(context)
{
}

const vk::Pipeline& c3d::VKPipeline::getHandle()
{
	return _pipeline;
}