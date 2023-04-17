#include "VKDescriptorSetLayout.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"

VKPtr<VKDescriptorSetLayout> VKDescriptorSetLayout::create(VKContext& context, const VKDescriptorSetLayoutInfo& info)
{
	return VKPtr<VKDescriptorSetLayout>(new VKDescriptorSetLayout(context, info));
}

VKDynamic<VKDescriptorSetLayout> VKDescriptorSetLayout::createDynamic(VKContext& context, const VKDescriptorSetLayoutInfo& info)
{
	return VKDynamic<VKDescriptorSetLayout>(context, info);
}

VKDescriptorSetLayout::VKDescriptorSetLayout(VKContext& context, const VKDescriptorSetLayoutInfo& info):
	VKObject(context), _info(info)
{
	std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(_info.getAllBindingInfos().size());
	std::vector<vk::DescriptorBindingFlags> vkBindingsFlags;
	vkBindingsFlags.reserve(_info.getAllBindingInfos().size());
	
	for (const auto& [binding, bindingInfo] : _info.getAllBindingInfos())
	{
		vk::DescriptorSetLayoutBinding& vkBinding = vkBindings.emplace_back();
		vkBinding.binding = binding;
		vkBinding.descriptorType = bindingInfo.type;
		vkBinding.descriptorCount = bindingInfo.count;
		vkBinding.stageFlags = vk::ShaderStageFlagBits::eAll;
		vkBinding.pImmutableSamplers = nullptr; // Optional
		
		vkBindingsFlags.emplace_back(bindingInfo.flags);
	}
	
	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
	bindingFlagsCreateInfo.bindingCount = vkBindingsFlags.size();
	bindingFlagsCreateInfo.pBindingFlags = vkBindingsFlags.data();
	
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.bindingCount = vkBindings.size();
	createInfo.pBindings = vkBindings.data();
	createInfo.pNext = &bindingFlagsCreateInfo;
	
	if (info.isPushable())
	{
		createInfo.flags |= vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
	}
	
	_handle = _context.getDevice().createDescriptorSetLayout(createInfo);
}

VKDescriptorSetLayout::~VKDescriptorSetLayout()
{
	_context.getDevice().destroyDescriptorSetLayout(_handle);
}

const VKDescriptorSetLayoutInfo& VKDescriptorSetLayout::getInfo() const
{
	return _info;
}

const vk::DescriptorSetLayout& VKDescriptorSetLayout::getHandle()
{
	return _handle;
}