#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class VKDescriptorSetLayout;
class VKBufferBase;
class VKSampler;
class VKImageView;
class VKAccelerationStructure;

class VKDescriptorSet : public VKObject
{
public:
	static VKPtr<VKDescriptorSet> create(VKContext& context, const VKDescriptorSetInfo& info);
	
	~VKDescriptorSet() override;
	
	const VKDescriptorSetInfo& getInfo() const;
	
	const vk::DescriptorSet& getHandle();
	
	void bindBuffer(uint32_t bindingIndex, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex = 0);
	void bindSampler(uint32_t bindingIndex, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindImage(uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex = 0);
	void bindCombinedImageSampler(uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindAccelerationStructure(uint32_t bindingIndex, const VKPtr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex = 0);
	
	void copyTo(uint32_t srcBindingIndex, uint32_t srcArrayIndex, const VKPtr<VKDescriptorSet>& dst, uint32_t dstBindingIndex, uint32_t dstArrayIndex, uint32_t count);
	
protected:
	VKDescriptorSet(VKContext& context, const VKDescriptorSetInfo& info);
	
	VKDescriptorSetInfo _info;
	
	vk::DescriptorPool _descriptorPool;
	vk::DescriptorSet _descriptorSet;
	
	// _boundObjects[bindingIndex][arrayIndex][object]
	// third vector is to support multiple VKObjects being bound to a single binding at the same time (e.g. combined image sampler)
	std::vector<std::vector<std::vector<VKPtr<VKObject>>>> _boundObjects;
};