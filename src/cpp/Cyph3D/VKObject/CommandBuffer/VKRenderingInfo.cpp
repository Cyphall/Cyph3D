#include "VKRenderingInfo.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

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

VKRenderingColorAttachmentInfo& VKRenderingInfo::addColorAttachment(const std::shared_ptr<VKImage>& image)
{
	return addColorAttachment(
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat()
	);
}

VKRenderingColorAttachmentInfo& VKRenderingInfo::addColorAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	return _colorAttachmentsInfos.emplace_back(image, type, layerRange, levelRange, format);
}

const std::vector<VKRenderingColorAttachmentInfo>& VKRenderingInfo::getColorAttachmentInfos() const
{
	return _colorAttachmentsInfos;
}

VKRenderingDepthAttachmentInfo& VKRenderingInfo::setDepthAttachment(const std::shared_ptr<VKImage>& image)
{
	return setDepthAttachment(
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat()
	);
}

VKRenderingDepthAttachmentInfo& VKRenderingInfo::setDepthAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_depthAttachmentInfo = std::make_optional<VKRenderingDepthAttachmentInfo>(image, type, layerRange, levelRange, format);
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