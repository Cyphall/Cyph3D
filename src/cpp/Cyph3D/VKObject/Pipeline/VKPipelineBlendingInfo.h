#pragma once

#include <vulkan/vulkan.hpp>

struct VKPipelineBlendingInfo
{
	vk::BlendFactor srcColorBlendFactor;
	vk::BlendFactor dstColorBlendFactor;
	vk::BlendOp colorBlendOp;
	vk::BlendFactor srcAlphaBlendFactor;
	vk::BlendFactor dstAlphaBlendFactor;
	vk::BlendOp alphaBlendOp;
};