#pragma once

#include <vulkan/vulkan.hpp>

namespace c3d
{
struct VKPipelineBlendingInfo
{
	vk::BlendFactor srcColorBlendFactor;
	vk::BlendFactor dstColorBlendFactor;
	vk::BlendOp colorBlendOp;
	vk::BlendFactor srcAlphaBlendFactor;
	vk::BlendFactor dstAlphaBlendFactor;
	vk::BlendOp alphaBlendOp;
};
}