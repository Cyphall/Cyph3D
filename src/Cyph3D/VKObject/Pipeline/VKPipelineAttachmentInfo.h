#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipelineBlendingInfo.h"

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

class VKPipelineAttachmentInfo
{
public:
	void registerColorAttachment(uint32_t slot, vk::Format format, std::optional<VKPipelineBlendingInfo> blending = std::nullopt);
	
	void setDepthAttachment(vk::Format format, vk::CompareOp depthTestPassCondition, bool writeEnabled);

private:
	friend class VKGraphicsPipeline;
	
	struct ColorAttachmentInfo
	{
		vk::Format format = vk::Format::eUndefined;
		std::optional<VKPipelineBlendingInfo> blending;
	};
	
	struct DepthAttachmentInfo
	{
		vk::Format format;
		vk::CompareOp depthTestPassCondition;
		bool writeEnabled;
	};
	
	std::vector<ColorAttachmentInfo> _colorAttachmentsInfo;
	
	DepthAttachmentInfo _depthAttachmentInfo = {vk::Format::eUndefined, vk::CompareOp::eAlways, false};
};