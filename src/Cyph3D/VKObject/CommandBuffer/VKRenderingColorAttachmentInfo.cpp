#include "VKRenderingColorAttachmentInfo.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

VKRenderingColorAttachmentInfo::VKRenderingColorAttachmentInfo(const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format):
	_imageInfo(image, type, layerRange, levelRange, format)
{

}

const VKRenderingColorAttachmentInfo::ImageInfo& VKRenderingColorAttachmentInfo::getImageInfo() const
{
	return _imageInfo;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image)
{
	return enableResolve(
		mode,
		image,
		image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
		{0, image->getInfo().getLayers() - 1},
		{0, image->getInfo().getLevels() - 1},
		image->getInfo().getFormat());
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	_resolveMode = mode;
	_resolveImageInfo = {image, type, layerRange, levelRange, format};
	
	return *this;
}

const vk::ResolveModeFlagBits& VKRenderingColorAttachmentInfo::getResolveMode() const
{
	return _resolveMode;
}

const VKRenderingColorAttachmentInfo::ImageInfo& VKRenderingColorAttachmentInfo::getResolveImageInfo() const
{
	return _resolveImageInfo;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setLoadOpLoad()
{
	_loadOp = vk::AttachmentLoadOp::eLoad;
	
	return *this;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setLoadOpClear(glm::vec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.float32[0] = clearValue.r;
	_clearValue.color.float32[1] = clearValue.g;
	_clearValue.color.float32[2] = clearValue.b;
	_clearValue.color.float32[3] = clearValue.a;
	
	return *this;
}


VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setLoadOpClear(glm::ivec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.int32[0] = clearValue.r;
	_clearValue.color.int32[1] = clearValue.g;
	_clearValue.color.int32[2] = clearValue.b;
	_clearValue.color.int32[3] = clearValue.a;
	
	return *this;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setLoadOpClear(glm::uvec4 clearValue)
{
	_loadOp = vk::AttachmentLoadOp::eClear;
	_clearValue.color.uint32[0] = clearValue.r;
	_clearValue.color.uint32[1] = clearValue.g;
	_clearValue.color.uint32[2] = clearValue.b;
	_clearValue.color.uint32[3] = clearValue.a;
	
	return *this;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setLoadOpDontCare()
{
	_loadOp = vk::AttachmentLoadOp::eDontCare;
	
	return *this;
}

const vk::AttachmentLoadOp& VKRenderingColorAttachmentInfo::getLoadOp() const
{
	return _loadOp;
}

const vk::ClearValue& VKRenderingColorAttachmentInfo::getClearValue() const
{
	return _clearValue;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setStoreOpStore()
{
	_storeOp = vk::AttachmentStoreOp::eStore;
	
	return *this;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setStoreOpDontCare()
{
	_storeOp = vk::AttachmentStoreOp::eDontCare;
	
	return *this;
}

VKRenderingColorAttachmentInfo& VKRenderingColorAttachmentInfo::setStoreOpNone()
{
	_storeOp = vk::AttachmentStoreOp::eNone;
	
	return *this;
}

const vk::AttachmentStoreOp& VKRenderingColorAttachmentInfo::getStoreOp() const
{
	return _storeOp;
}