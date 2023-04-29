#include "VKDescriptorSetLayoutInfo.h"

VKDescriptorSetLayoutInfo::VKDescriptorSetLayoutInfo(bool pushable):
	_pushable(pushable)
{

}

void VKDescriptorSetLayoutInfo::registerBinding(uint32_t binding, vk::DescriptorType type, uint32_t count)
{
	auto it = _bindings.try_emplace(binding);
	if (!it.second)
	{
		throw;
	}
	
	VKDescriptorSetLayoutBinding& descriptorSetLayoutBinding = it.first->second;
	descriptorSetLayoutBinding.type = type;
	descriptorSetLayoutBinding.count = count;
	descriptorSetLayoutBinding.flags = {};
}

void VKDescriptorSetLayoutInfo::registerIndexedBinding(uint32_t binding, vk::DescriptorType type, uint32_t upperBound)
{
	auto it = _bindings.try_emplace(binding);
	if (!it.second)
	{
		throw;
	}
	
	VKDescriptorSetLayoutBinding& descriptorSetLayoutBinding = it.first->second;
	descriptorSetLayoutBinding.type = type;
	descriptorSetLayoutBinding.count = upperBound;
	descriptorSetLayoutBinding.flags = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
}

const bool& VKDescriptorSetLayoutInfo::isPushable() const
{
	return _pushable;
}

const VKDescriptorSetLayoutBinding& VKDescriptorSetLayoutInfo::getBindingInfo(uint32_t binding) const
{
	return _bindings.at(binding);
}

const std::unordered_map<uint32_t, VKDescriptorSetLayoutBinding>& VKDescriptorSetLayoutInfo::getAllBindingInfos() const
{
	return _bindings;
}
