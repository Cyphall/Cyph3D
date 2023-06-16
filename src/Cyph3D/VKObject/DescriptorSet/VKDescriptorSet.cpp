#include "VKDescriptorSet.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKHelper.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
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

VKDescriptorSet::VKDescriptorSet(VKContext& context, const VKDescriptorSetInfo& info):
	VKObject(context), _info(info)
{
	vk::DescriptorSetVariableDescriptorCountAllocateInfo descriptorSetVariableDescriptorCountAllocateInfo;
	descriptorSetVariableDescriptorCountAllocateInfo.descriptorSetCount = 0;
	descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts = nullptr;
	
	_boundObjects.resize(_info.getLayout()->getInfo().getBindingInfos().size());
	
	std::vector<vk::DescriptorPoolSize> requiredResources;
	bool anyBindingHasUpdateAfterBind = false;
	for (uint32_t i = 0; i < _info.getLayout()->getInfo().getBindingInfos().size(); i++)
	{
		const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(i);
		
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
		
		_boundObjects[i].resize(bindingInfo.count);
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

void VKDescriptorSet::bindBuffer(uint32_t bindingIndex, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex)
{
	vk::DescriptorBufferInfo bufferInfo;
	if (size > 0)
	{
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset * buffer->getStride();
		bufferInfo.range = size * buffer->getStride();
		
		_boundObjects[bindingIndex][arrayIndex] = {buffer};
	}
	else
	{
		bufferInfo.buffer = VK_NULL_HANDLE;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
	}
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pTexelBufferView = nullptr; // Optional
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects[bindingIndex][arrayIndex] = {buffer};
}

void VKDescriptorSet::bindSampler(uint32_t bindingIndex, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	vk::DescriptorImageInfo samplerInfo;
	samplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &samplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects[bindingIndex][arrayIndex] = {sampler};
}

void VKDescriptorSet::bindImage(uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex)
{
	VKHelper::assertImageViewHasUniqueLayout(imageView);
	
	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView->getHandle();
	imageInfo.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects[bindingIndex][arrayIndex] = {imageView};
}

void VKDescriptorSet::bindCombinedImageSampler(uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	VKHelper::assertImageViewHasUniqueLayout(imageView);
	
	vk::DescriptorImageInfo combinedImageSamplerInfo;
	combinedImageSamplerInfo.imageView = imageView->getHandle();
	combinedImageSamplerInfo.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	combinedImageSamplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = _descriptorSet;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &combinedImageSamplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects[bindingIndex][arrayIndex] = {imageView, sampler};
}

void VKDescriptorSet::bindAccelerationStructure(uint32_t bindingIndex, const VKPtr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex)
{
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptorWrite;
	accelerationStructureDescriptorWrite.accelerationStructureCount = 1;
	accelerationStructureDescriptorWrite.pAccelerationStructures = &accelerationStructure->getHandle();
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	descriptorWrite.pNext = &accelerationStructureDescriptorWrite;
	
	_context.getDevice().updateDescriptorSets(descriptorWrite, nullptr);
	
	_boundObjects[bindingIndex][arrayIndex] = {accelerationStructure};
}