#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "glm/fwd.hpp"

#include <vk_mem_alloc.hpp>
#include <optional>
#include <array>

class VKImage;

class VKImageView : public VKObject
{
public:
	static VKPtr<VKImageView> create(
		VKContext& context,
		const VKPtr<VKImage>& image,
		vk::ImageViewType viewType,
		std::optional<std::array<vk::ComponentSwizzle, 4>> swizzle = std::nullopt,
		std::optional<vk::Format> viewFormat = std::nullopt,
		std::optional<glm::uvec2> referencedLayerRange = std::nullopt,
		std::optional<glm::uvec2> referencedLevelRange = std::nullopt);
	static VKDynamic<VKImageView> createDynamic(
		VKContext& context,
		const VKDynamic<VKImage>& image,
		vk::ImageViewType viewType,
		std::optional<std::array<vk::ComponentSwizzle, 4>> swizzle = std::nullopt,
		std::optional<vk::Format> viewFormat = std::nullopt,
		std::optional<glm::uvec2> referencedLayerRange = std::nullopt,
		std::optional<glm::uvec2> referencedLevelRange = std::nullopt);
	
	~VKImageView() override;
	
	const VKPtr<VKImage>& getImage();
	
	vk::ImageView& getHandle();
	
	uint32_t getFirstReferencedLayer() const;
	uint32_t getLastReferencedLayer() const;
	
	uint32_t getFirstReferencedLevel() const;
	uint32_t getLastReferencedLevel() const;

private:
	VKImageView(
		VKContext& context,
		const VKPtr<VKImage>& image,
		vk::ImageViewType viewType,
		std::optional<std::array<vk::ComponentSwizzle, 4>> swizzle,
		std::optional<vk::Format> viewFormat,
		std::optional<glm::uvec2> referencedLayerRange,
		std::optional<glm::uvec2> referencedLevelRange);
	
	vk::ImageView _handle;
	VKPtr<VKImage> _image;
	glm::uvec2 _referencedLayerRange;
	glm::uvec2 _referencedLevelRange;
};