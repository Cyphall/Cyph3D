#include "VKComputePipelineInfo.h"

VKComputePipelineInfo::VKComputePipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout, const std::filesystem::path& computeShader):
	_pipelineLayout(pipelineLayout),
	_computeShader(computeShader)
{
}

const std::shared_ptr<VKPipelineLayout>& VKComputePipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::filesystem::path& VKComputePipelineInfo::getComputeShader() const
{
	return _computeShader;
}
