#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKImage;

class VKRenderingDepthAttachmentInfo
{
public:
	struct ImageInfo
	{
		VKPtr<VKImage> image;
		vk::ImageViewType type;
		glm::uvec2 layerRange;
		glm::uvec2 levelRange;
		vk::Format format;
	};
	
	VKRenderingDepthAttachmentInfo(const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);
	
	const ImageInfo& getImageInfo() const;
	
	VKRenderingDepthAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image);
	VKRenderingDepthAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);
	const vk::ResolveModeFlagBits& getResolveMode() const;
	const ImageInfo& getResolveImageInfo() const;
	
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
	ImageInfo _imageInfo;
	vk::ResolveModeFlagBits _resolveMode = vk::ResolveModeFlagBits::eNone;
	ImageInfo _resolveImageInfo;
	vk::AttachmentLoadOp _loadOp = vk::AttachmentLoadOp::eDontCare;
	vk::ClearValue _clearValue;
	vk::AttachmentStoreOp _storeOp = vk::AttachmentStoreOp::eDontCare;
};