#include "VKComputePipelineInfo.h"

c3d::VKComputePipelineInfo::VKComputePipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout, const std::string& computeShader):
	_pipelineLayout(pipelineLayout),
	_computeShader(computeShader)
{
}

const std::shared_ptr<c3d::VKPipelineLayout>& c3d::VKComputePipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::string& c3d::VKComputePipelineInfo::getComputeShader() const
{
	return _computeShader;
}