#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <optional>

class VKImage;

class VKImageViewInfo
{
public:
	explicit VKImageViewInfo(const VKPtr<VKImage>& image, vk::ImageViewType viewType);
	
	const VKPtr<VKImage>& getImage() const;
	
	const vk::ImageViewType& getViewType() const;
	
	void setSwizzle(std::array<vk::ComponentSwizzle, 4> swizzle);
	bool hasSwizzle() const;
	const std::array<vk::ComponentSwizzle, 4>& getSwizzle() const;
	
	void setCustomViewFormat(vk::Format viewFormat);
	bool hasCustomViewFormat() const;
	const vk::Format& getCustomViewFormat() const;
	
	void setCustomLayerRange(glm::uvec2 layerRange);
	bool hasCustomLayerRange() const;
	const glm::uvec2& getCustomLayerRange() const;
	
	void setCustomLevelRange(glm::uvec2 levelRange);
	bool hasCustomLevelRange() const;
	const glm::uvec2& getCustomLevelRange() const;

private:
	VKPtr<VKImage> _image;
	vk::ImageViewType _viewType;
	std::optional<std::array<vk::ComponentSwizzle, 4>> _swizzle;
	std::optional<vk::Format> _customViewFormat;
	std::optional<glm::uvec2> _customLayerRange;
	std::optional<glm::uvec2> _customLevelRange;
};