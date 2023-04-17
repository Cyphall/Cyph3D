#pragma once

#include <vulkan/vulkan.hpp>

struct VKDescriptorSetLayoutBinding
{
	vk::DescriptorType type;
	uint32_t count;
	vk::DescriptorBindingFlags flags;
};