#include "VKDescriptorSetInfo.h"

c3d::VKDescriptorSetInfo::VKDescriptorSetInfo(const std::shared_ptr<VKDescriptorSetLayout>& layout):
	_layout(layout)
{
}

const uint32_t& c3d::VKDescriptorSetInfo::getVariableSizeAllocatedCount() const
{
	return _variableSizeAllocatedCount;
}

void c3d::VKDescriptorSetInfo::setVariableSizeAllocatedCount(uint32_t count)
{
	_variableSizeAllocatedCount = count;
}

const std::shared_ptr<c3d::VKDescriptorSetLayout>& c3d::VKDescriptorSetInfo::getLayout() const
{
	return _layout;
}