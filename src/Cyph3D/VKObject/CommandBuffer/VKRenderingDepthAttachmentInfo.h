#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class VKImageView;

class VKRenderingDepthAttachmentInfo
{
public:
	explicit VKRenderingDepthAttachmentInfo(const VKPtr<VKImageView>& imageView);
	
	const VKPtr<VKImageView>& getImageView() const;
	
	VKRenderingDepthAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImageView>& imageView);
	const vk::ResolveModeFlagBits& getResolveMode() const;
	const VKPtr<VKImageView>& getResolveImageView() const;
	
	VKRenderingDepthAttachmentInfo& setLoadOpLoad();
	VKRenderingDepthAttachmentInfo& setLoadOpClear(float clearValue);
	VKRenderingDepthAttachmentInfo& setLoadOpDontCare();
	const vk::AttachmentLoadOp& getLoadOp() const;
	const vk::ClearValue& getClearValue() const;
	
	VKRenderingDepthAttachmentInfo& setStoreOpStore();
	VKRenderingDepthAttachmentInfo& setStoreOpDontCare();
	VKRenderingDepthAttachmentInfo& setStoreOpNone();
	const vk::AttachmentStoreOp& getStoreOp() const;

private:
	VKPtr<VKImageView> _imageView;
	vk::ResolveModeFlagBits _resolveMode = vk::ResolveModeFlagBits::eNone;
	VKPtr<VKImageView> _resolveImageView;
	vk::AttachmentLoadOp _loadOp = vk::AttachmentLoadOp::eDontCare;
	vk::AttachmentStoreOp _storeOp = vk::AttachmentStoreOp::eDontCare;
	vk::ClearValue _clearValue;
};