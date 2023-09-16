#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipelineBlendingInfo.h"

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>

class VKPipelineAttachmentInfo
{
public:
	struct ColorAttachmentInfo
	{
		vk::Format format;
		std::optional<VKPipelineBlendingInfo> blending;
	};
	
	struct DepthAttachmentInfo
	{
		vk::Format format;
		vk::CompareOp depthTestPassCondition;
		bool writeEnabled;
	};
	
	const ColorAttachmentInfo& getColorAttachmentInfo(uint32_t attachmentLocation) const;
	const std::vector<ColorAttachmentInfo>& getColorAttachmentsInfos() const;
	void addColorAttachment(vk::Format format, std::optional<VKPipelineBlendingInfo> blending = std::nullopt);
	
	bool hasDepthAttachment() const;
	const DepthAttachmentInfo& getDepthAttachmentInfo() const;
	void setDepthAttachment(vk::Format format, vk::CompareOp depthTestPassCondition, bool writeEnabled);

private:
	std::vector<ColorAttachmentInfo> _colorAttachmentsInfos;
	
	std::optional<DepthAttachmentInfo> _depthAttachmentInfo;
};