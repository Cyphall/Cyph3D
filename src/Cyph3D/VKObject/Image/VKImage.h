#pragma once

#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/VKObject/Image/VKImageInfo.h"
#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

class VKImage : public VKObject
{
public:
	struct State
	{
		vk::ImageLayout layout;
		vk::PipelineStageFlags2 stageMask;
		vk::AccessFlags2 accessMask;

		bool operator==(const State& other) const = default;
	};

	static VKPtr<VKImage> create(VKContext& context, const VKImageInfo& info);

	~VKImage() override;

	const VKImageInfo& getInfo() const;

	const vk::Image& getHandle();

	const glm::uvec2& getSize(uint32_t level) const;

	const State& getState(uint32_t layer, uint32_t level) const;

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

	vk::Image _handle;
	VmaAllocation _allocation = VK_NULL_HANDLE;

	std::vector<glm::uvec2> _sizes;
	std::vector<std::vector<State>> _currentStates;
	std::unordered_map<ViewInfo, vk::ImageView, ViewInfoHasher> _views;

	VKImage(VKContext& context, const VKImageInfo& info);

	void setState(glm::uvec2 layerRange, glm::uvec2 levelRange, const State& state);
};