#include "VKDescriptorSetLayoutInfo.h"

c3d::VKDescriptorSetLayoutInfo::VKDescriptorSetLayoutInfo(bool pushable):
	_pushable(pushable)
{
}

const bool& c3d::VKDescriptorSetLayoutInfo::isPushable() const
{
	return _pushable;
}

const c3d::VKDescriptorSetLayoutInfo::BindingInfo& c3d::VKDescriptorSetLayoutInfo::getBindingInfo(uint32_t bindingIndex) const
{
	return _bindingInfos[bindingIndex];
}

const std::vector<c3d::VKDescriptorSetLayoutInfo::BindingInfo>& c3d::VKDescriptorSetLayoutInfo::getBindingInfos() const
{
	return _bindingInfos;
}

void c3d::VKDescriptorSetLayoutInfo::addBinding(vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags shaderStages)
{
	VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _bindingInfos.emplace_back();
	bindingInfo.type = type;
	bindingInfo.count = count;
	bindingInfo.flags = {};
	bindingInfo.shaderStages = shaderStages;
}

void c3d::VKDescriptorSetLayoutInfo::addIndexedBinding(vk::DescriptorType type, uint32_t upperBound, vk::ShaderStageFlags shaderStages)
{
	VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _bindingInfos.emplace_back();
	bindingInfo.type = type;
	bindingInfo.count = upperBound;
	bindingInfo.flags = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
	bindingInfo.shaderStages = shaderStages;
}