#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKImage;

class VKRenderingColorAttachmentInfo
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

	VKRenderingColorAttachmentInfo(const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);

	const ImageInfo& getImageInfo() const;

	VKRenderingColorAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image);
	VKRenderingColorAttachmentInfo& enableResolve(vk::ResolveModeFlagBits mode, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);
	const vk::ResolveModeFlagBits& getResolveMode() const;
	const ImageInfo& getResolveImageInfo() const;

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
	ImageInfo _imageInfo;
	vk::ResolveModeFlagBits _resolveMode = vk::ResolveModeFlagBits::eNone;
	ImageInfo _resolveImageInfo{};
	vk::AttachmentLoadOp _loadOp = vk::AttachmentLoadOp::eDontCare;
	vk::ClearValue _clearValue;
	vk::AttachmentStoreOp _storeOp = vk::AttachmentStoreOp::eDontCare;
};