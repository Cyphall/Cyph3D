#pragma once

#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

class VKDescriptorSetLayout;
class VKBufferBase;
class VKSampler;
class VKImage;
class VKAccelerationStructure;

class VKDescriptorSet : public VKObject
{
public:
	static std::shared_ptr<VKDescriptorSet> create(VKContext& context, const VKDescriptorSetInfo& info);

	~VKDescriptorSet() override;

	const VKDescriptorSetInfo& getInfo() const;

	const vk::DescriptorSet& getHandle();

	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindDescriptor(uint32_t bindingIndex, const std::shared_ptr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex = 0);

	void copyTo(uint32_t srcBindingIndex, uint32_t srcArrayIndex, const std::shared_ptr<VKDescriptorSet>& dst, uint32_t dstBindingIndex, uint32_t dstArrayIndex, uint32_t count);

protected:
	VKDescriptorSet(VKContext& context, const VKDescriptorSetInfo& info);

	VKDescriptorSetInfo _info;

	vk::DescriptorPool _descriptorPool;
	vk::DescriptorSet _descriptorSet;

	// _boundObjects[bindingIndex][arrayIndex][object]
	// third vector is to support multiple VKObjects being bound to a single binding at the same time (e.g. combined image sampler)
	std::vector<std::vector<std::vector<std::shared_ptr<VKObject>>>> _boundObjects;
};