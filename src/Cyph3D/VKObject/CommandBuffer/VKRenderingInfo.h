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
	
	VKRenderingColorAttachmentInfo& addColorAttachment(const VKPtr<VKImageView>& imageView);
	const std::vector<VKRenderingColorAttachmentInfo>& getColorAttachmentInfos() const;
	
	VKRenderingDepthAttachmentInfo& setDepthAttachment(const VKPtr<VKImageView>& imageView);
	bool hasDepthAttachment() const;
	const VKRenderingDepthAttachmentInfo& getDepthAttachmentInfo() const;

private:
	glm::uvec2 _size;
	uint32_t _layers = 1;
	std::vector<VKRenderingColorAttachmentInfo> _colorAttachmentsInfos;
	std::optional<VKRenderingDepthAttachmentInfo> _depthAttachmentInfo;
};