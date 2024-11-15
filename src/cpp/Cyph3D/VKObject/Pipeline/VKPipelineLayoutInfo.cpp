#include "VKPipelineLayoutInfo.h"

#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"

void VKPipelineLayoutInfo::addDescriptorSetLayout(const std::shared_ptr<VKDescriptorSetLayout>& descriptorSetLayout)
{
	_descriptorSetsLayouts.push_back(descriptorSetLayout);
}

const std::shared_ptr<VKDescriptorSetLayout>& VKPipelineLayoutInfo::getDescriptorSetLayout(uint32_t setIndex) const
{
	return _descriptorSetsLayouts[setIndex];
}

const std::vector<std::shared_ptr<VKDescriptorSetLayout>>& VKPipelineLayoutInfo::getDescriptorSetLayouts() const
{
	return _descriptorSetsLayouts;
}

const std::optional<VKPipelineLayoutInfo::PushConstantInfo>& VKPipelineLayoutInfo::getPushConstantInfo() const
{
	return _pushConstantInfo;
}