#include "VKComputePipelineInfo.h"

VKComputePipelineInfo::VKComputePipelineInfo(const VKPtr<VKPipelineLayout>& pipelineLayout, const std::filesystem::path& computeShader):
	_pipelineLayout(pipelineLayout), _computeShader(computeShader)
{

}

const VKPtr<VKPipelineLayout>& VKComputePipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::filesystem::path& VKComputePipelineInfo::getComputeShader() const
{
	return _computeShader;
}
