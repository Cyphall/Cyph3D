#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <cstdint>

class VKDescriptorSetLayout;

class VKDescriptorSetInfo
{
public:
	explicit VKDescriptorSetInfo(const VKPtr<VKDescriptorSetLayout>& layout);
	
	const uint32_t& getVariableSizeAllocatedCount() const;
	void setVariableSizeAllocatedCount(uint32_t count);
	
	const VKPtr<VKDescriptorSetLayout>& getLayout() const;
	
private:
	VKPtr<VKDescriptorSetLayout> _layout;
	uint32_t _variableSizeAllocatedCount = 0;
};