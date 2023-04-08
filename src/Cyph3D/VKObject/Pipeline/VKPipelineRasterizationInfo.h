#pragma once

#include <vulkan/vulkan.hpp>

struct VKPipelineRasterizationInfo
{
	vk::CullModeFlags cullMode;
	vk::FrontFace frontFace;
};