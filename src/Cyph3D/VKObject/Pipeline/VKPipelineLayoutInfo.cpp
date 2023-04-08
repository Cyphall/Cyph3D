#include "VKPipelineLayoutInfo.h"

#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"

void VKPipelineLayoutInfo::registerDescriptorSetLayout(const VKPtr<VKDescriptorSetLayout>& descriptorSetLayout)
{
	_vkDescriptorSetsLayouts.push_back(descriptorSetLayout->getHandle());
	_descriptorSetsLayouts.push_back(descriptorSetLayout);
}