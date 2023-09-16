#pragma once

#include "Cyph3D/VKObject/Image/VKImageInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

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
	
	static int calcMaxMipLevels(const glm::uvec2& size);
	
private:
	friend class VKCommandBuffer;
	
	VKImageInfo _info;
	
	vma::Allocation _imageAlloc;
	
	vk::Image _handle;
	
	std::vector<glm::uvec2> _sizes;
	std::vector<std::vector<vk::ImageLayout>> _currentLayouts;
	
	VKImage(VKContext& context, const VKImageInfo& info);
	
	void setLayout(uint32_t layer, uint32_t level, vk::ImageLayout layout);
};