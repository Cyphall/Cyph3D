#include "VKPipelineAttachmentInfo.h"

const c3d::VKPipelineAttachmentInfo::ColorAttachmentInfo& c3d::VKPipelineAttachmentInfo::getColorAttachmentInfo(uint32_t attachmentLocation) const
{
	return _colorAttachmentsInfos[attachmentLocation];
}

const std::vector<c3d::VKPipelineAttachmentInfo::ColorAttachmentInfo>& c3d::VKPipelineAttachmentInfo::getColorAttachmentsInfos() const
{
	return _colorAttachmentsInfos;
}

void c3d::VKPipelineAttachmentInfo::addColorAttachment(vk::Format format, std::optional<VKPipelineBlendingInfo> blending)
{
	ColorAttachmentInfo& colorAttachmentInfo = _colorAttachmentsInfos.emplace_back();
	colorAttachmentInfo.format = format;
	colorAttachmentInfo.blending = blending;
}

bool c3d::VKPipelineAttachmentInfo::hasDepthAttachment() const
{
	return _depthAttachmentInfo.has_value();
}

const c3d::VKPipelineAttachmentInfo::DepthAttachmentInfo& c3d::VKPipelineAttachmentInfo::getDepthAttachmentInfo() const
{
	return _depthAttachmentInfo.value();
}

void c3d::VKPipelineAttachmentInfo::setDepthAttachment(vk::Format format, vk::CompareOp depthTestPassCondition, bool writeEnabled)
{
	_depthAttachmentInfo = DepthAttachmentInfo{
		.format = format,
		.depthTestPassCondition = depthTestPassCondition,
		.writeEnabled = writeEnabled
	};
}