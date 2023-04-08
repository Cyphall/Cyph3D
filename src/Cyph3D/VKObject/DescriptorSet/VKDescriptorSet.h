#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>

class VKDescriptorSetLayout;
class VKBufferBase;
class VKSampler;
class VKImageView;

class VKDescriptorSet : public VKObject
{
public:
	static VKPtr<VKDescriptorSet> create(VKContext& context, const VKDescriptorSetInfo& info);
	static VKDynamic<VKDescriptorSet> createDynamic(VKContext& context, const VKDescriptorSetInfo& info);
	
	~VKDescriptorSet() override;
	
	const VKDescriptorSetInfo& getInfo() const;
	
	const vk::DescriptorSet& getHandle();
	
	void bindBuffer(uint32_t binding, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex = 0);
	void bindSampler(uint32_t binding, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void bindImage(uint32_t binding, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex = 0);
	void bindCombinedImageSampler(uint32_t binding, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	
protected:
	VKDescriptorSet(VKContext& context, const VKDescriptorSetInfo& info);
	
	VKDescriptorSetInfo _info;
	
	vk::DescriptorPool _descriptorPool;
	vk::DescriptorSet _descriptorSet;
	
	std::unordered_map<uint32_t, std::vector<VKPtr<VKObject>>> _boundObjects;
};