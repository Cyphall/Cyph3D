#pragma once


#include <optional>
#include <vector>
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

	void addDescriptorSetLayout(const std::shared_ptr<VKDescriptorSetLayout>& descriptorSetLayout);

	template<typename T>
	void setPushConstantLayout(vk::ShaderStageFlags shaderStages = vk::ShaderStageFlagBits::eAll)
	{
		_pushConstantInfo = PushConstantInfo{
			.size = sizeof(T),
			.shaderStages = shaderStages
		};
	}

	const std::shared_ptr<VKDescriptorSetLayout>& getDescriptorSetLayout(uint32_t setIndex) const;
	const std::vector<std::shared_ptr<VKDescriptorSetLayout>>& getDescriptorSetLayouts() const;

	const std::optional<PushConstantInfo>& getPushConstantInfo() const;

private:
	std::vector<std::shared_ptr<VKDescriptorSetLayout>> _descriptorSetsLayouts;
	std::optional<PushConstantInfo> _pushConstantInfo;
};