#include "VKDescriptorSetLayoutInfo.h"

VKDescriptorSetLayoutInfo::VKDescriptorSetLayoutInfo(bool pushable):
	_pushable(pushable)
{

}

const bool& VKDescriptorSetLayoutInfo::isPushable() const
{
	return _pushable;
}

const VKDescriptorSetLayoutInfo::BindingInfo& VKDescriptorSetLayoutInfo::getBindingInfo(uint32_t bindingIndex) const
{
	return _bindingInfos[bindingIndex];
}

const std::vector<VKDescriptorSetLayoutInfo::BindingInfo>& VKDescriptorSetLayoutInfo::getBindingInfos() const
{
	return _bindingInfos;
}

void VKDescriptorSetLayoutInfo::addBinding(vk::DescriptorType type, uint32_t count)
{
	VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _bindingInfos.emplace_back();
	bindingInfo.type = type;
	bindingInfo.count = count;
	bindingInfo.flags = {};
}

void VKDescriptorSetLayoutInfo::addIndexedBinding(vk::DescriptorType type, uint32_t upperBound)
{
	VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _bindingInfos.emplace_back();
	bindingInfo.type = type;
	bindingInfo.count = upperBound;
	bindingInfo.flags = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
}