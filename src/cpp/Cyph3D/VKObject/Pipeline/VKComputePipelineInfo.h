#pragma once

#include <memory>
#include <string>

namespace c3d
{
class VKPipelineLayout;

class VKComputePipelineInfo
{
public:
	VKComputePipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout, const std::string& computeShader);

	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const;

	const std::string& getComputeShader() const;

private:
	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::string _computeShader;
};
}