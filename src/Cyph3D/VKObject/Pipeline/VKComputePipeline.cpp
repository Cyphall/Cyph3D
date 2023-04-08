#include "VKComputePipeline.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Shader/VKShader.h"
#include "Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.h"

VKPtr<VKComputePipeline> VKComputePipeline::create(VKContext& context, VKComputePipelineInfo& info)
{
	return VKPtr<VKComputePipeline>(new VKComputePipeline(context, info));
}

VKDynamic<VKComputePipeline> VKComputePipeline::createDynamic(VKContext& context, VKComputePipelineInfo& info)
{
	return VKDynamic<VKComputePipeline>(context, info);
}

VKComputePipeline::VKComputePipeline(VKContext& context, VKComputePipelineInfo& info):
	VKPipeline(context, info.pipelineLayout)
{
	createPipeline(info);
}

VKComputePipeline::~VKComputePipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

void VKComputePipeline::createPipeline(VKComputePipelineInfo& info)
{
	VKPtr<VKShader> shader = VKShader::create(_context, info.computeShaderFile);
	
	vk::PipelineShaderStageCreateInfo createInfo;
	createInfo.stage = vk::ShaderStageFlagBits::eVertex;
	createInfo.module = shader->getHandle();
	createInfo.pName = "main";
	
	vk::ComputePipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.stage = createInfo;
	pipelineCreateInfo.layout = _pipelineLayout->getHandle();
	
	_pipeline = _context.getDevice().createComputePipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
}

vk::PipelineBindPoint VKComputePipeline::getPipelineType()
{
	return vk::PipelineBindPoint::eCompute;
}