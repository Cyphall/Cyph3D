#include "VKRenderingInfo.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

c3d::VKRenderingInfo::VKRenderingInfo(glm::uvec2 size):
	_size(size)
{
}

const glm::uvec2& c3d::VKRenderingInfo::getSize() const
{
	return _size;
}

void c3d::VKRenderingInfo::setLayers(uint32_t layers)
{
	_layers = layers;
}

const uint32_t& c3d::VKRenderingInfo::getLayers() const
{
	return _layers;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingInfo::addColorAttachment(const std::shared_ptr<VKImage>& image)
{
	return addColorAttachment(
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat()
	);
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingInfo::addColorAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	return _colorAttachmentsInfos.emplace_back(image, type, layerRange, levelRange, format);
}

const std::vector<c3d::VKRenderingColorAttachmentInfo>& c3d::VKRenderingInfo::getColorAttachmentInfos() const
{
	return _colorAttachmentsInfos;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingInfo::setDepthAttachment(const std::shared_ptr<VKImage>& image)
{
	return setDepthAttachment(
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat()
	);
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingInfo::setDepthAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_depthAttachmentInfo = std::make_optional<VKRenderingDepthAttachmentInfo>(image, type, layerRange, levelRange, format);
	return _depthAttachmentInfo.value();
}

bool c3d::VKRenderingInfo::hasDepthAttachment() const
{
	return _depthAttachmentInfo.has_value();
}

const c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingInfo::getDepthAttachmentInfo() const
{
	return _depthAttachmentInfo.value();
}