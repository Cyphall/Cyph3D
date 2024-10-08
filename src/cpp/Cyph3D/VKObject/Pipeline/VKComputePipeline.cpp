#include "VKComputePipeline.h"

#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Shader/VKShader.h"
#include "Cyph3D/VKObject/VKContext.h"

std::shared_ptr<VKComputePipeline> VKComputePipeline::create(VKContext& context, const VKComputePipelineInfo& info)
{
	return std::shared_ptr<VKComputePipeline>(new VKComputePipeline(context, info));
}

VKComputePipeline::VKComputePipeline(VKContext& context, const VKComputePipelineInfo& info):
	VKPipeline(context),
	_info(info)
{
	std::shared_ptr<VKShader> shader = VKShader::create(_context, _info.getComputeShader());

	vk::PipelineShaderStageCreateInfo createInfo;
	createInfo.stage = vk::ShaderStageFlagBits::eCompute;
	createInfo.module = shader->getHandle();
	createInfo.pName = "main";

	vk::ComputePipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.stage = createInfo;
	pipelineCreateInfo.layout = _info.getPipelineLayout()->getHandle();

	_pipeline = _context.getDevice().createComputePipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
}

VKComputePipeline::~VKComputePipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

const VKComputePipelineInfo& VKComputePipeline::getInfo() const
{
	return _info;
}

vk::PipelineBindPoint VKComputePipeline::getPipelineType() const
{
	return vk::PipelineBindPoint::eCompute;
}

const std::shared_ptr<VKPipelineLayout>& VKComputePipeline::getPipelineLayout() const
{
	return _info.getPipelineLayout();
}