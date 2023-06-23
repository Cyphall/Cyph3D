#include "VKRenderingInfo.h"

VKRenderingInfo::VKRenderingInfo(glm::uvec2 size):
	_size(size)
{

}

const glm::uvec2& VKRenderingInfo::getSize() const
{
	return _size;
}

void VKRenderingInfo::setLayers(uint32_t layers)
{
	_layers = layers;
}

const uint32_t& VKRenderingInfo::getLayers() const
{
	return _layers;
}

VKRenderingColorAttachmentInfo& VKRenderingInfo::addColorAttachment(const VKPtr<VKImageView>& imageView)
{
	return _colorAttachmentsInfos.emplace_back(imageView);
}

const std::vector<VKRenderingColorAttachmentInfo>& VKRenderingInfo::getColorAttachmentInfos() const
{
	return _colorAttachmentsInfos;
}

VKRenderingDepthAttachmentInfo& VKRenderingInfo::setDepthAttachment(const VKPtr<VKImageView>& imageView)
{
	_depthAttachmentInfo = std::make_optional<VKRenderingDepthAttachmentInfo>(imageView);
	return _depthAttachmentInfo.value();
}

bool VKRenderingInfo::hasDepthAttachment() const
{
	return _depthAttachmentInfo.has_value();
}

const VKRenderingDepthAttachmentInfo& VKRenderingInfo::getDepthAttachmentInfo() const
{
	return _depthAttachmentInfo.value();
}