#pragma once

#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/VKObject/Image/VKImageInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>

class VKImage : public VKObject
{
public:
	static VKPtr<VKImage> create(VKContext& context, const VKImageInfo& info);

	~VKImage() override;

	const VKImageInfo& getInfo() const;

	const vk::Image& getHandle();

	const glm::uvec2& getSize(uint32_t level) const;

	vk::ImageLayout getLayout(uint32_t layer, uint32_t level) const;

	vk::DeviceSize getLayerByteSize() const;
	vk::DeviceSize getLevelByteSize(uint32_t level) const;
	vk::DeviceSize getPixelByteSize() const;

	bool isCompressed() const;

	vk::ImageView getView(vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format);

	static int calcMaxMipLevels(const glm::uvec2& size);

private:
	friend class VKCommandBuffer;

	struct ViewInfo
	{
		vk::ImageViewType type;
		glm::uvec2 layerRange;
		glm::uvec2 levelRange;
		vk::Format format;

		bool operator==(const ViewInfo& other) const = default;
	};

	struct ViewInfoHasher
	{
		std::size_t operator()(const VKImage::ViewInfo& key) const
		{
			return HashBuilder()
				.hash(key.type)
				.hash(key.layerRange)
				.hash(key.levelRange)
				.hash(key.format)
				.get();
		}
	};

	VKImageInfo _info;

	vma::Allocation _imageAlloc;

	vk::Image _handle;

	std::vector<glm::uvec2> _sizes;
	std::vector<std::vector<vk::ImageLayout>> _currentLayouts;
	std::unordered_map<ViewInfo, vk::ImageView, ViewInfoHasher> _views;

	VKImage(VKContext& context, const VKImageInfo& info);

	void setLayout(glm::uvec2 layerRange, glm::uvec2 levelRange, vk::ImageLayout layout);
};