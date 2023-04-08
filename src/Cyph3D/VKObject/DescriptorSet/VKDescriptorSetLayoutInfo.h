#pragma once

#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutBinding.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>

class VKDescriptorSetLayoutInfo
{
public:
	explicit VKDescriptorSetLayoutInfo(bool pushable);
	
	void registerBinding(uint32_t binding, vk::DescriptorType type, uint32_t count);
	void registerIndexedBinding(uint32_t binding, vk::DescriptorType type, uint32_t upperBound);
	
	const bool& isPushable() const;
	
	const VKDescriptorSetLayoutBinding& getBindingInfo(uint32_t binding) const;
	const std::unordered_map<uint32_t, VKDescriptorSetLayoutBinding>& getAllBindingInfos() const;
	
private:
	std::unordered_map<uint32_t, VKDescriptorSetLayoutBinding> _bindings;
	bool _pushable;
};