#include "VKDescriptorSetInfo.h"

VKDescriptorSetInfo::VKDescriptorSetInfo(const VKPtr<VKDescriptorSetLayout>& layout):
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

const VKPtr<VKDescriptorSetLayout>& VKDescriptorSetInfo::getLayout() const
{
	return _layout;
}
