#include "VKDescriptorSetLayout.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"

VKPtr<VKDescriptorSetLayout> VKDescriptorSetLayout::create(VKContext& context, const VKDescriptorSetLayoutInfo& info)
{
	return VKPtr<VKDescriptorSetLayout>(new VKDescriptorSetLayout(context, info));
}

VKDescriptorSetLayout::VKDescriptorSetLayout(VKContext& context, const VKDescriptorSetLayoutInfo& info):
	VKObject(context), _info(info)
{
	std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(_info.getBindingInfos().size());
	std::vector<vk::DescriptorBindingFlags> vkBindingsFlags;
	vkBindingsFlags.reserve(_info.getBindingInfos().size());
	
	bool anyBindingHasUpdateAfterBind = false;
	for (uint32_t i = 0; i < _info.getBindingInfos().size(); i++)
	{
		const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getBindingInfo(i);
		
		vk::DescriptorSetLayoutBinding& vkBinding = vkBindings.emplace_back();
		vkBinding.binding = i;
		vkBinding.descriptorType = bindingInfo.type;
		vkBinding.descriptorCount = bindingInfo.count;
		vkBinding.stageFlags = vk::ShaderStageFlagBits::eAll;
		vkBinding.pImmutableSamplers = nullptr;
		
		vkBindingsFlags.emplace_back(bindingInfo.flags);
		
		if (bindingInfo.flags & vk::DescriptorBindingFlagBits::eUpdateAfterBind)
		{
			anyBindingHasUpdateAfterBind = true;
		}
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
	
	if (anyBindingHasUpdateAfterBind)
	{
		createInfo.flags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
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