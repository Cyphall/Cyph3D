#pragma once


#include <cstdint>

class VKDescriptorSetLayout;

class VKDescriptorSetInfo
{
public:
	explicit VKDescriptorSetInfo(const std::shared_ptr<VKDescriptorSetLayout>& layout);

	const uint32_t& getVariableSizeAllocatedCount() const;
	void setVariableSizeAllocatedCount(uint32_t count);

	const std::shared_ptr<VKDescriptorSetLayout>& getLayout() const;

private:
	std::shared_ptr<VKDescriptorSetLayout> _layout;
	uint32_t _variableSizeAllocatedCount = 0;
};