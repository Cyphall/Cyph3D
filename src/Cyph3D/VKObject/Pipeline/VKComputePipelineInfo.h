#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>

class VKPipelineLayout;

struct VKComputePipelineInfo
{
	std::filesystem::path computeShaderFile;
	VKPtr<VKPipelineLayout> pipelineLayout;
};