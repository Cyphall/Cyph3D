#include "VKPipelineAttachmentInfo.h"

void VKPipelineAttachmentInfo::addColorAttachment(uint32_t slot, vk::Format format, std::optional<VKPipelineBlendingInfo> blending)
{
	_colorAttachmentsInfo.resize(std::max<size_t>(slot + 1, _colorAttachmentsInfo.size()), {});
	
	ColorAttachmentInfo& colorAttachmentInfo = _colorAttachmentsInfo[slot];
	colorAttachmentInfo.format = format;
	colorAttachmentInfo.blending = blending;
}

void VKPipelineAttachmentInfo::setDepthAttachment(vk::Format format, vk::CompareOp depthTestPassCondition, bool writeEnabled)
{
	_depthAttachmentInfo.format = format;
	_depthAttachmentInfo.depthTestPassCondition = depthTestPassCondition;
	_depthAttachmentInfo.writeEnabled = writeEnabled;
}