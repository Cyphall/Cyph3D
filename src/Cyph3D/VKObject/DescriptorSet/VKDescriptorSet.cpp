#include "VKDescriptorSet.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

VKPtr<VKDescriptorSet> VKDescriptorSet::create(VKContext& context, const VKDescriptorSetInfo& info)
{
	return VKPtr<VKDescriptorSet>(new VKDescriptorSet(context, info));
}

VKDynamic<VKDescriptorSet> VKDescriptorSet::createDynamic(VKContext& context, const VKDescriptorSetInfo& info)
{
	return VKDynamic<VKDescriptorSet>(context, info);
}

VKDescriptorSet::VKDescriptorSet(VKContext& context, const VKDescriptorSetInfo& info):
	VKObject(context), _info(info)
{
	vk::DescriptorSetVariableDescriptorCountAllocateInfo descriptorSetVariableDescriptorCountAllocateInfo;
	descriptorSetVariableDescriptorCountAllocateInfo.descriptorSetCount = 0;
	descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts = nullptr;
	
	std::vector<vk::DescriptorPoolSize> requiredResources;
	bool anyBindingHasUpdateAfterBind = false;
	for (const auto& [binding, bindingInfo] : _info.getLayout()->getInfo().getAllBindingInfos())
	{
		vk::DescriptorPoolSize& descriptorPoolSize = requiredResources.emplace_back();
		descriptorPoolSize.type = bindingInfo.type;
		descriptorPoolSize.descriptorCount = bindingInfo.count;
		
		if (bindingInfo.flags & vk::DescriptorBindingFlagBits::eVariableDescriptorCount)
		{
			descriptorSetVariableDescriptorCountAllocateInfo.descriptorSetCount = 1;
			descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts = &_info.getVariableSizeAllocatedCount();
		}
		
		if (bindingInfo.flags & vk::DescriptorBindingFlagBits::eUpdateAfterBind)
		{
			anyBindingHasUpdateAfterBind = true;
		}
		
		auto [it, inserted] = _boundObjects.try_emplace(binding);
		it->second.resize(bindingInfo.count);
	}
	
	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.poolSizeCount = requiredResources.size();
	poolInfo.pPoolSizes = requiredResources.data();
	poolInfo.maxSets = 1;
	
	if (anyBindingHasUpdateAfterBind)
	{
		poolInfo.flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
	}
	
	_descriptorPool = _context.getDevice().createDescriptorPool(poolInfo);
	
	vk::DescriptorSetLayout vkLayout = _info.getLayout()->getHandle();
	
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkLayout;
	allocInfo.pNext = &descriptorSetVariableDescriptorCountAllocateInfo;
	
	_descriptorSet = _context.getDevice().allocateDescriptorSets(allocInfo).front();
}

VKDescriptorSet::~VKDescriptorSet()
{
	_context.getDevice().destroyDescriptorPool(_descriptorPool);
}

const VKDescriptorSetInfo& VKDescriptorSet::getInfo() const
{
	return _info;
}

const vk::DescriptorSet& VKDescriptorSet::getHandle()
{
	return _descriptorSet;
}

void VKDescriptorSet::bindBuffer(uint32_t binding, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex)
{
	vk::DescriptorBufferInfo bufferInfo;
	if (size > 0)
	{
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset * buffer->getStride();
		bufferInfo.range = size * buffer->getStride();
		
		_boundObjects.at(binding).at(arrayIndex) = buffer;
	}
	else
	{
		bufferInfo.buffer = VK_NULL_HANDLE;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
	}
	
	const VKDescriptorSetLayoutBinding& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(binding);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pTexelBufferView = nullptr; // Optional
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
}

void VKDescriptorSet::bindSampler(uint32_t binding, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	vk::DescriptorImageInfo samplerInfo;
	samplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutBinding& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(binding);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &samplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects.at(binding).at(arrayIndex) = sampler;
}

void VKDescriptorSet::bindImage(uint32_t binding, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex)
{
	// make sure all referenced layers and levels have the same layout
	const VKPtr<VKImage>& image = imageView->getImage();
	vk::ImageLayout layout = image->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	for (uint32_t layer = imageView->getFirstReferencedLayer(); layer <= imageView->getLastReferencedLayer(); layer++)
	{
		for (uint32_t level = imageView->getFirstReferencedLevel(); level <= imageView->getLastReferencedLevel(); level++)
		{
			if (image->getLayout(layer, level) != layout)
			{
				throw;
			}
		}
	}
	
	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView->getHandle();
	imageInfo.imageLayout = layout;
	
	const VKDescriptorSetLayoutBinding& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(binding);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects.at(binding).at(arrayIndex) = image;
}

void VKDescriptorSet::bindCombinedImageSampler(uint32_t binding, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	// make sure all referenced layers and levels have the same layout
	const VKPtr<VKImage>& image = imageView->getImage();
	vk::ImageLayout layout = image->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	for (uint32_t layer = imageView->getFirstReferencedLayer(); layer <= imageView->getLastReferencedLayer(); layer++)
	{
		for (uint32_t level = imageView->getFirstReferencedLevel(); level <= imageView->getLastReferencedLevel(); level++)
		{
			if (image->getLayout(layer, level) != layout)
			{
				throw;
			}
		}
	}
	
	vk::DescriptorImageInfo combinedImageSamplerInfo;
	combinedImageSamplerInfo.imageView = imageView->getHandle();
	combinedImageSamplerInfo.imageLayout = layout;
	combinedImageSamplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutBinding& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(binding);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &combinedImageSamplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects.at(binding).at(arrayIndex) = image;
}