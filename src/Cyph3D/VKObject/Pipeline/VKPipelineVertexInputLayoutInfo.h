#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class VKPipelineVertexInputLayoutInfo
{
public:
	void defineAttribute(uint32_t bufferSlot, uint32_t attributeLocation, vk::Format dataType, uint32_t offset);
	void defineSlot(uint32_t bufferSlot, int stride, vk::VertexInputRate inputRate);

	vk::PipelineVertexInputStateCreateInfo get() const;

private:
	std::vector<vk::VertexInputAttributeDescription> _attributes;
	std::vector<vk::VertexInputBindingDescription> _slots;
};