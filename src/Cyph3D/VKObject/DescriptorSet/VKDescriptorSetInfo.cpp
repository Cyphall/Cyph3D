#include "VKDescriptorSetInfo.h"

VKDescriptorSetInfo::VKDescriptorSetInfo(const std::shared_ptr<VKDescriptorSetLayout>& layout):
	_layout(layout)
{
}

const uint32_t& VKDescriptorSetInfo::getVariableSizeAllocatedCount() const
{
	return _variableSizeAllocatedCount;
}

void VKDescriptorSetInfo::setVariableSizeAllocatedCount(uint32_t count)
{
	_variableSizeAllocatedCount = count;
}

const std::shared_ptr<VKDescriptorSetLayout>& VKDescriptorSetInfo::getLayout() const
{
	return _layout;
}
