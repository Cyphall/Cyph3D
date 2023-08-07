#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

class VKDescriptorSetLayout;

class VKPipelineLayoutInfo
{
public:
	struct PushConstantInfo
	{
		uint32_t size;
		vk::ShaderStageFlags shaderStages;
	};
	
	void addDescriptorSetLayout(const VKPtr<VKDescriptorSetLayout>& descriptorSetLayout);
	
	template<typename T>
	void setPushConstantLayout(vk::ShaderStageFlags shaderStages = vk::ShaderStageFlagBits::eAll)
	{
		_pushConstantInfo = PushConstantInfo{
			.size = sizeof(T),
			.shaderStages = shaderStages
		};
	}
	
	const VKPtr<VKDescriptorSetLayout>& getDescriptorSetLayout(uint32_t setIndex) const;
	const std::vector<VKPtr<VKDescriptorSetLayout>>& getDescriptorSetLayouts() const;
	
	const std::optional<PushConstantInfo>& getPushConstantInfo() const;
	
private:
	std::vector<VKPtr<VKDescriptorSetLayout>> _descriptorSetsLayouts;
	std::optional<PushConstantInfo> _pushConstantInfo;
};