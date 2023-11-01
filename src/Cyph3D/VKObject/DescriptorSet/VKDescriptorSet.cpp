#include "VKDescriptorSet.h"

#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKHelper.h"

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
			
			_boundObjects[i].resize(*descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts);
		}
		else
		{
			
			_boundObjects[i].resize(bindingInfo.count);
		}
		
		if (bindingInfo.flags & vk::DescriptorBindingFlagBits::eUpdateAfterBind)
		{
			anyBindingHasUpdateAfterBind = true;
		}
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

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex)
{
	if (buffer && size > 0)
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset * buffer->getStride();
		bufferInfo.range = size * buffer->getStride();
		
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
	else
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = VK_NULL_HANDLE;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		
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
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	if (sampler)
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
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKImage>& image, uint32_t arrayIndex)
{
	if (image)
	{
		bindDescriptor(
			bindingIndex,
			image,
			image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
			{0, image->getInfo().getLayers() - 1},
			{0, image->getInfo().getLevels() - 1},
			image->getInfo().getFormat(),
			arrayIndex);
	}
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, uint32_t arrayIndex)
{
	if (image)
	{
		VKHelper::assertImageViewHasUniqueLayout(image, layerRange, levelRange);
		
		vk::DescriptorImageInfo imageInfo;
		imageInfo.imageView = image->getView(type, layerRange, levelRange, format);
		imageInfo.imageLayout = image->getLayout(layerRange.x, levelRange.x);
		
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
		
		_boundObjects[bindingIndex][arrayIndex] = {image};
	}
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKImage>& image, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	if (image && sampler)
	{
		bindDescriptor(
			bindingIndex,
			image,
			image->getInfo().isCubeCompatible() ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
			{0, image->getInfo().getLayers() - 1},
			{0, image->getInfo().getLevels() - 1},
			image->getInfo().getFormat(),
			sampler,
			arrayIndex);
	}
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	if (image && sampler)
	{
		VKHelper::assertImageViewHasUniqueLayout(image, layerRange, levelRange);
		
		vk::DescriptorImageInfo combinedImageSamplerInfo;
		combinedImageSamplerInfo.imageView = image->getView(type, layerRange, levelRange, format);
		combinedImageSamplerInfo.imageLayout = image->getLayout(layerRange.x, levelRange.x);
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
		
		_boundObjects[bindingIndex][arrayIndex] = {image, sampler};
	}
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::bindDescriptor(uint32_t bindingIndex, const VKPtr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex)
{
	if (accelerationStructure)
	{
		const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _info.getLayout()->getInfo().getBindingInfo(bindingIndex);
		
		vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptorWrite;
		accelerationStructureDescriptorWrite.accelerationStructureCount = 1;
		accelerationStructureDescriptorWrite.pAccelerationStructures = &accelerationStructure->getHandle();
		
		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = _descriptorSet;
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
	else
	{
		_boundObjects[bindingIndex][arrayIndex] = {};
	}
}

void VKDescriptorSet::copyTo(uint32_t srcBindingIndex, uint32_t srcArrayIndex, const VKPtr<VKDescriptorSet>& dst, uint32_t dstBindingIndex, uint32_t dstArrayIndex, uint32_t count)
{
	vk::CopyDescriptorSet copyDescriptorSet;
	copyDescriptorSet.srcSet = _descriptorSet;
	copyDescriptorSet.srcBinding = srcBindingIndex;
	copyDescriptorSet.srcArrayElement = srcArrayIndex;
	copyDescriptorSet.dstSet = dst->_descriptorSet;
	copyDescriptorSet.dstBinding = dstBindingIndex;
	copyDescriptorSet.dstArrayElement = dstArrayIndex;
	copyDescriptorSet.descriptorCount = count;
	
	_context.getDevice().updateDescriptorSets({}, copyDescriptorSet);
	
	for (uint32_t i = 0; i < count; i++)
	{
		dst->_boundObjects[dstBindingIndex][dstArrayIndex + i] = _boundObjects[srcBindingIndex][srcArrayIndex + i];
	}
}
