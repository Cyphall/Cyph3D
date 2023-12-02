#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>

class VKPipelineLayout;

class VKComputePipelineInfo
{
public:
	VKComputePipelineInfo(const VKPtr<VKPipelineLayout>& pipelineLayout, const std::filesystem::path& computeShader);

	const VKPtr<VKPipelineLayout>& getPipelineLayout() const;

	const std::filesystem::path& getComputeShader() const;

private:
	VKPtr<VKPipelineLayout> _pipelineLayout;
	std::filesystem::path _computeShader;
};