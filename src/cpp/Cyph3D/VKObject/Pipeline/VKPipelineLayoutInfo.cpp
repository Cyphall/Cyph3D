#include "VKPipelineLayoutInfo.h"

#include <Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h>

void c3d::VKPipelineLayoutInfo::addDescriptorSetLayout(const std::shared_ptr<VKDescriptorSetLayout>& descriptorSetLayout)
{
	_descriptorSetsLayouts.push_back(descriptorSetLayout);
}

const std::shared_ptr<c3d::VKDescriptorSetLayout>& c3d::VKPipelineLayoutInfo::getDescriptorSetLayout(uint32_t setIndex) const
{
	return _descriptorSetsLayouts[setIndex];
}

const std::vector<std::shared_ptr<c3d::VKDescriptorSetLayout>>& c3d::VKPipelineLayoutInfo::getDescriptorSetLayouts() const
{
	return _descriptorSetsLayouts;
}

const std::optional<c3d::VKPipelineLayoutInfo::PushConstantInfo>& c3d::VKPipelineLayoutInfo::getPushConstantInfo() const
{
	return _pushConstantInfo;
}