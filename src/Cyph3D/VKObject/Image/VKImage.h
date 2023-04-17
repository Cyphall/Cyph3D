#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vk_mem_alloc.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKImage : public VKObject
{
public:
	static VKPtr<VKImage> create(
		VKContext& context,
		vk::Format format,
		const glm::uvec2& size,
		uint32_t layers,
		uint32_t levels,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlagBits aspect,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {},
		bool isCubeCompatible = false,
		const vk::ArrayProxy<vk::Format>& possibleViewFormats = {});
	static VKDynamic<VKImage> createDynamic(
		VKContext& context,
		vk::Format format,
		const glm::uvec2& size,
		uint32_t layers,
		uint32_t levels,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlagBits aspect,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {},
		bool isCubeCompatible = false,
		const vk::ArrayProxy<vk::Format>& possibleViewFormats = {});
	
	~VKImage() override;
	
	const vk::Image& getHandle();
	
	vk::Format getFormat() const;
	
	const glm::uvec2& getSize(uint32_t level) const;
	
	vk::ImageLayout getLayout(uint32_t layer, uint32_t level) const;
	
	vk::ImageAspectFlags getAspect() const;
	
	uint32_t getLayers() const;
	uint32_t getLevels() const;
	
	vk::DeviceSize getLayerByteSize() const;
	vk::DeviceSize getLevelByteSize(uint32_t level) const;
	vk::DeviceSize getPixelByteSize() const;
	
	static int calcMaxMipLevels(const glm::uvec2& size);
	
protected:
	VKImage(
		VKContext& context,
		vk::Image handle,
		vk::Format format,
		const glm::uvec2& size,
		uint32_t layers,
		uint32_t levels,
		vk::ImageAspectFlagBits aspect);
	
private:
	friend class VKCommandBuffer;
	
	vma::Allocation _imageAlloc;
	
	bool _ownsHandle;
	
	vk::Image _handle;
	
	vk::Format _format;
	std::vector<glm::uvec2> _sizes;
	std::vector<std::vector<vk::ImageLayout>> _currentLayouts;
	uint32_t _layers;
	uint32_t _levels;
	vk::ImageAspectFlags _aspect;
	
	VKImage(
		VKContext& context,
		vk::Format format,
		const glm::uvec2& size,
		uint32_t layers,
		uint32_t levels,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlagBits aspect,
		vk::MemoryPropertyFlags requiredProperties,
		vk::MemoryPropertyFlags preferredProperties,
		bool isCubeCompatible,
		const vk::ArrayProxy<vk::Format>& possibleViewFormats);
	
	void initLayoutsAndSizes(glm::uvec2 size);
	
	void setLayout(vk::ImageLayout layout, uint32_t layer, uint32_t level);
};