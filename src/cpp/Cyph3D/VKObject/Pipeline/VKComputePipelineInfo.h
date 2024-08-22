#pragma once


#include <filesystem>

class VKPipelineLayout;

class VKComputePipelineInfo
{
public:
	VKComputePipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout, const std::filesystem::path& computeShader);

	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const;

	const std::filesystem::path& getComputeShader() const;

private:
	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::filesystem::path _computeShader;
};