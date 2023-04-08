#include "VKPipelineVertexInputLayoutInfo.h"

void VKPipelineVertexInputLayoutInfo::defineAttribute(uint32_t bufferSlot, uint32_t attributeLocation, vk::Format dataType, uint32_t offset)
{
	vk::VertexInputAttributeDescription& attributeDescription = _attributes.emplace_back();
	attributeDescription.binding = bufferSlot;
	attributeDescription.location = attributeLocation;
	attributeDescription.format = dataType;
	attributeDescription.offset = offset;
}

void VKPipelineVertexInputLayoutInfo::defineSlot(uint32_t bufferSlot, int stride, vk::VertexInputRate inputRate)
{
	vk::VertexInputBindingDescription& slotDescription = _slots.emplace_back();
	slotDescription.binding = bufferSlot;
	slotDescription.stride = stride;
	slotDescription.inputRate = inputRate;
}

vk::PipelineVertexInputStateCreateInfo VKPipelineVertexInputLayoutInfo::get() const
{
	vk::PipelineVertexInputStateCreateInfo description;
	description.vertexBindingDescriptionCount = _slots.size();
	description.pVertexBindingDescriptions = _slots.data();
	description.vertexAttributeDescriptionCount = _attributes.size();
	description.pVertexAttributeDescriptions = _attributes.data();
	
	return description;
}