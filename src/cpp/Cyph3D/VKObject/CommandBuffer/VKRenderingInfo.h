#pragma once

#include "Cyph3D/VKObject/CommandBuffer/VKRenderingColorAttachmentInfo.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingDepthAttachmentInfo.h"

#include <optional>

class VKRenderingInfo
{
public:
	explicit VKRenderingInfo(glm::uvec2 size);

	const glm::uvec2& getSize() const;

	void setLayers(uint32_t layers);
	const uint32_t& getLayers() const;

	VKRenderingColorAttachmentInfo& addColorAttachment(const std::shared_ptr<VKImage>& image);
	VKRenderingColorAttachmentInfo& addColorAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);
	const std::vector<VKRenderingColorAttachmentInfo>& getColorAttachmentInfos() const;

	VKRenderingDepthAttachmentInfo& setDepthAttachment(const std::shared_ptr<VKImage>& image);
	VKRenderingDepthAttachmentInfo& setDepthAttachment(const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);
	bool hasDepthAttachment() const;
	const VKRenderingDepthAttachmentInfo& getDepthAttachmentInfo() const;

private:
	glm::uvec2 _size;
	uint32_t _layers = 1;
	std::vector<VKRenderingColorAttachmentInfo> _colorAttachmentsInfos;
	std::optional<VKRenderingDepthAttachmentInfo> _depthAttachmentInfo;
};