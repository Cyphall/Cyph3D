#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineViewport.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineScissor.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineRasterizationInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.h"

#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <optional>

class VKPipelineLayout;

struct VKGraphicsPipelineInfo
{
	std::optional<std::filesystem::path> vertexShaderFile;
	std::optional<std::filesystem::path> geometryShaderFile;
	std::optional<std::filesystem::path> fragmentShaderFile;
	VKPipelineVertexInputLayoutInfo vertexInputLayoutInfo;
	vk::PrimitiveTopology vertexTopology;
	VKPtr<VKPipelineLayout> pipelineLayout;
	std::optional<VKPipelineViewport> viewport;
	std::optional<VKPipelineScissor> scissor;
	VKPipelineRasterizationInfo rasterizationInfo;
	VKPipelineAttachmentInfo pipelineAttachmentInfo;
};