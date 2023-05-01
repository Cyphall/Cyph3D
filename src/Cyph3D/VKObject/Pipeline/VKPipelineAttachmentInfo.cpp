#include "VKPipelineAttachmentInfo.h"

const VKPipelineAttachmentInfo::ColorAttachmentInfo& VKPipelineAttachmentInfo::getColorAttachmentInfo(uint32_t attachmentLocation) const
{
	return _colorAttachmentsInfos[attachmentLocation];
}

const std::vector<VKPipelineAttachmentInfo::ColorAttachmentInfo>& VKPipelineAttachmentInfo::getColorAttachmentsInfos() const
{
	return _colorAttachmentsInfos;
}

void VKPipelineAttachmentInfo::addColorAttachment(vk::Format format, std::optional<VKPipelineBlendingInfo> blending)
{
	ColorAttachmentInfo& colorAttachmentInfo = _colorAttachmentsInfos.emplace_back();
	colorAttachmentInfo.format = format;
	colorAttachmentInfo.blending = blending;
}

bool VKPipelineAttachmentInfo::hasDepthAttachment() const
{
	return _depthAttachmentInfo.has_value();
}

const VKPipelineAttachmentInfo::DepthAttachmentInfo& VKPipelineAttachmentInfo::getDepthAttachmentInfo() const
{
	return _depthAttachmentInfo.value();
}

void VKPipelineAttachmentInfo::setDepthAttachment(vk::Format format, vk::CompareOp depthTestPassCondition, bool writeEnabled)
{
	_depthAttachmentInfo = DepthAttachmentInfo{
		.format = format,
		.depthTestPassCondition = depthTestPassCondition,
		.writeEnabled = writeEnabled
	};
}

void VKPipelineAttachmentInfo::unsetDepthAttachment()
{
	_depthAttachmentInfo = std::nullopt;
}