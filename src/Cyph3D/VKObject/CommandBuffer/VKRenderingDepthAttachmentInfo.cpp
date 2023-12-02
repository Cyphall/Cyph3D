#include "VKRenderingDepthAttachmentInfo.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

VKRenderingDepthAttachmentInfo::VKRenderingDepthAttachmentInfo(const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format):
	_imageInfo(image, type, layerRange, levelRange, format)
{

}

const VKRenderingDepthAttachmentInfo::ImageInfo& VKRenderingDepthAttachmentInfo::getImageInfo() const
{
	return _imageInfo;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image)
{
	return enableResolve(
		mode,
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat());
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_resolveMode = mode;
	_resolveImageInfo = {image, type, layerRange, levelRange, format};

	return *this;
}

const vk::ResolveModeFlagBits& VKRenderingDepthAttachmentInfo::getResolveMode() const
{
	return _resolveMode;
}

const VKRenderingDepthAttachmentInfo::ImageInfo& VKRenderingDepthAttachmentInfo::getResolveImageInfo() const
{
	return _resolveImageInfo;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setLoadOpLoad()
{
	_loadOp = vk::AttachmentLoadOp::eLoad;

	return *this;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setLoadOpClear(float clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.depthStencil.depth = clearValue;

	return *this;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setLoadOpDontCare()
{
	_loadOp = vk::AttachmentLoadOp::eDontCare;

	return *this;
}

const vk::AttachmentLoadOp& VKRenderingDepthAttachmentInfo::getLoadOp() const
{
	return _loadOp;
}

const vk::ClearValue& VKRenderingDepthAttachmentInfo::getClearValue() const
{
	return _clearValue;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setStoreOpStore()
{
	_storeOp = vk::AttachmentStoreOp::eStore;

	return *this;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setStoreOpDontCare()
{
	_storeOp = vk::AttachmentStoreOp::eDontCare;

	return *this;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::setStoreOpNone()
{
	_storeOp = vk::AttachmentStoreOp::eNone;

	return *this;
}

const vk::AttachmentStoreOp& VKRenderingDepthAttachmentInfo::getStoreOp() const
{
	return _storeOp;
}