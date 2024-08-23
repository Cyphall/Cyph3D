#include "VKComputePipelineInfo.h"

VKComputePipelineInfo::VKComputePipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout, const std::string& computeShader):
	_pipelineLayout(pipelineLayout),
	_computeShader(computeShader)
{
}

const std::shared_ptr<VKPipelineLayout>& VKComputePipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::string& VKComputePipelineInfo::getComputeShader() const
{
	return _computeShader;
}
