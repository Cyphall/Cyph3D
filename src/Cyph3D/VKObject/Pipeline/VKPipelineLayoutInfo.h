#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vector>
#include <optional>

class VKDescriptorSetLayout;

class VKPipelineLayoutInfo
{
public:
	struct PushConstantInfo
	{
		uint32_t size;
	};
	
	void addDescriptorSetLayout(const VKPtr<VKDescriptorSetLayout>& descriptorSetLayout);
	
	template<typename T>
	void setPushConstantLayout()
	{
		_pushConstantInfo = PushConstantInfo{
			.size = sizeof(T)
		};
	}
	
	const VKPtr<VKDescriptorSetLayout>& getDescriptorSetLayout(uint32_t setIndex) const;
	const std::vector<VKPtr<VKDescriptorSetLayout>>& getDescriptorSetLayouts() const;
	
	const std::optional<PushConstantInfo>& getPushConstantInfo() const;
	
private:
	std::vector<VKPtr<VKDescriptorSetLayout>> _descriptorSetsLayouts;
	std::optional<PushConstantInfo> _pushConstantInfo;
};