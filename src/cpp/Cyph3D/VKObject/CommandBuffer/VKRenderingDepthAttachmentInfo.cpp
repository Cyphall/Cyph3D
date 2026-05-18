#include "VKRenderingDepthAttachmentInfo.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

c3d::VKRenderingDepthAttachmentInfo::VKRenderingDepthAttachmentInfo(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format):
	_imageInfo(image, type, layerRange, levelRange, format)
{
}

const c3d::VKRenderingDepthAttachmentInfo::ImageInfo& c3d::VKRenderingDepthAttachmentInfo::getImageInfo() const
{
	return _imageInfo;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const std::shared_ptr<VKImage>& image)
{
	return enableResolve(
		mode,
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat()
	);
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_resolveMode = mode;
	_resolveImageInfo = {image, type, layerRange, levelRange, format};

	return *this;
}

const vk::ResolveModeFlagBits& c3d::VKRenderingDepthAttachmentInfo::getResolveMode() const
{
	return _resolveMode;
}

const c3d::VKRenderingDepthAttachmentInfo::ImageInfo& c3d::VKRenderingDepthAttachmentInfo::getResolveImageInfo() const
{
	return _resolveImageInfo;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setLoadOpLoad()
{
	_loadOp = vk::AttachmentLoadOp::eLoad;

	return *this;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setLoadOpClear(float clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.depthStencil.depth = clearValue;

	return *this;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setLoadOpDontCare()
{
	_loadOp = vk::AttachmentLoadOp::eDontCare;

	return *this;
}

const vk::AttachmentLoadOp& c3d::VKRenderingDepthAttachmentInfo::getLoadOp() const
{
	return _loadOp;
}

const vk::ClearValue& c3d::VKRenderingDepthAttachmentInfo::getClearValue() const
{
	return _clearValue;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setStoreOpStore()
{
	_storeOp = vk::AttachmentStoreOp::eStore;

	return *this;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setStoreOpDontCare()
{
	_storeOp = vk::AttachmentStoreOp::eDontCare;

	return *this;
}

c3d::VKRenderingDepthAttachmentInfo& c3d::VKRenderingDepthAttachmentInfo::setStoreOpNone()
{
	_storeOp = vk::AttachmentStoreOp::eNone;

	return *this;
}

const vk::AttachmentStoreOp& c3d::VKRenderingDepthAttachmentInfo::getStoreOp() const
{
	return _storeOp;
}