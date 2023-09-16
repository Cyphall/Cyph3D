#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKImageView;

class VKRenderingColorAttachmentInfo
{
public:
	explicit VKRenderingColorAttachmentInfo(const VKPtr<VKImageView>& imageView);
	
	const VKPtr<VKImageView>& getImageView() const;
	
	VKRenderingColorAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImageView>& imageView);
	const vk::ResolveModeFlagBits& getResolveMode() const;
	const VKPtr<VKImageView>& getResolveImageView() const;
	
	VKRenderingColorAttachmentInfo& setLoadOpLoad();
	VKRenderingColorAttachmentInfo& setLoadOpClear(glm::vec4 clearValue);
	VKRenderingColorAttachmentInfo& setLoadOpClear(glm::ivec4 clearValue);
	VKRenderingColorAttachmentInfo& setLoadOpClear(glm::uvec4 clearValue);
	VKRenderingColorAttachmentInfo& setLoadOpDontCare();
	const vk::AttachmentLoadOp& getLoadOp() const;
	const vk::ClearValue& getClearValue() const;
	
	VKRenderingColorAttachmentInfo& setStoreOpStore();
	VKRenderingColorAttachmentInfo& setStoreOpDontCare();
	VKRenderingColorAttachmentInfo& setStoreOpNone();
	const vk::AttachmentStoreOp& getStoreOp() const;

private:
	VKPtr<VKImageView> _imageView;
	vk::ResolveModeFlagBits _resolveMode = vk::ResolveModeFlagBits::eNone;
	VKPtr<VKImageView> _resolveImageView;
	vk::AttachmentLoadOp _loadOp = vk::AttachmentLoadOp::eDontCare;
	vk::ClearValue _clearValue;
	vk::AttachmentStoreOp _storeOp = vk::AttachmentStoreOp::eDontCare;
};