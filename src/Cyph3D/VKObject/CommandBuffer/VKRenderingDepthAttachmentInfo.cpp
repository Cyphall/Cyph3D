#include "VKRenderingDepthAttachmentInfo.h"

VKRenderingDepthAttachmentInfo::VKRenderingDepthAttachmentInfo(const VKPtr<VKImageView>& imageView):
	_imageView(imageView)
{
	
}

const VKPtr<VKImageView>& VKRenderingDepthAttachmentInfo::getImageView() const
{
	return _imageView;
}

VKRenderingDepthAttachmentInfo& VKRenderingDepthAttachmentInfo::enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImageView>& imageView)
{
	_resolveMode = mode;
	_resolveImageView = imageView;
	
	return *this;
}

const vk::ResolveModeFlagBits& VKRenderingDepthAttachmentInfo::getResolveMode() const
{
	return _resolveMode;
}

const VKPtr<VKImageView>& VKRenderingDepthAttachmentInfo::getResolveImageView() const
{
	return _resolveImageView;
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