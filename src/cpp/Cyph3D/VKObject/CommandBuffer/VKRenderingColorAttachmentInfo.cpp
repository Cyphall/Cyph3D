#include "VKRenderingColorAttachmentInfo.h"

#include <Cyph3D/VKObject/Image/VKImage.h>

c3d::VKRenderingColorAttachmentInfo::VKRenderingColorAttachmentInfo(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format):
	_imageInfo(image, type, layerRange, levelRange, format)
{
}

const c3d::VKRenderingColorAttachmentInfo::ImageInfo& c3d::VKRenderingColorAttachmentInfo::getImageInfo() const
{
	return _imageInfo;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const std::shared_ptr<VKImage>& image)
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

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_resolveMode = mode;
	_resolveImageInfo = {image, type, layerRange, levelRange, format};

	return *this;
}

const vk::ResolveModeFlagBits& c3d::VKRenderingColorAttachmentInfo::getResolveMode() const
{
	return _resolveMode;
}

const c3d::VKRenderingColorAttachmentInfo::ImageInfo& c3d::VKRenderingColorAttachmentInfo::getResolveImageInfo() const
{
	return _resolveImageInfo;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setLoadOpLoad()
{
	_loadOp = vk::AttachmentLoadOp::eLoad;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setLoadOpClear(glm::vec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.float32[0] = clearValue.r;
	_clearValue.color.float32[1] = clearValue.g;
	_clearValue.color.float32[2] = clearValue.b;
	_clearValue.color.float32[3] = clearValue.a;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setLoadOpClear(glm::ivec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.int32[0] = clearValue.r;
	_clearValue.color.int32[1] = clearValue.g;
	_clearValue.color.int32[2] = clearValue.b;
	_clearValue.color.int32[3] = clearValue.a;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setLoadOpClear(glm::uvec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.uint32[0] = clearValue.r;
	_clearValue.color.uint32[1] = clearValue.g;
	_clearValue.color.uint32[2] = clearValue.b;
	_clearValue.color.uint32[3] = clearValue.a;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setLoadOpDontCare()
{
	_loadOp = vk::AttachmentLoadOp::eDontCare;

	return *this;
}

const vk::AttachmentLoadOp& c3d::VKRenderingColorAttachmentInfo::getLoadOp() const
{
	return _loadOp;
}

const vk::ClearValue& c3d::VKRenderingColorAttachmentInfo::getClearValue() const
{
	return _clearValue;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setStoreOpStore()
{
	_storeOp = vk::AttachmentStoreOp::eStore;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setStoreOpDontCare()
{
	_storeOp = vk::AttachmentStoreOp::eDontCare;

	return *this;
}

c3d::VKRenderingColorAttachmentInfo& c3d::VKRenderingColorAttachmentInfo::setStoreOpNone()
{
	_storeOp = vk::AttachmentStoreOp::eNone;

	return *this;
}

const vk::AttachmentStoreOp& c3d::VKRenderingColorAttachmentInfo::getStoreOp() const
{
	return _storeOp;
}