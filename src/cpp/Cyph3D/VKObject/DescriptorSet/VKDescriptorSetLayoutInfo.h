#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class VKDescriptorSetLayoutInfo
{
public:
	struct BindingInfo
	{
		vk::DescriptorType type;
		uint32_t count;
		vk::DescriptorBindingFlags flags;
		vk::ShaderStageFlags shaderStages;
	};

	explicit VKDescriptorSetLayoutInfo(bool pushable);

	const bool& isPushable() const;

	const BindingInfo& getBindingInfo(uint32_t bindingIndex) const;
	const std::vector<BindingInfo>& getBindingInfos() const;
	void addBinding(vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags shaderStages = vk::ShaderStageFlagBits::eAll);
	void addIndexedBinding(vk::DescriptorType type, uint32_t upperBound, vk::ShaderStageFlags shaderStages = vk::ShaderStageFlagBits::eAll);

private:
	bool _pushable;
	std::vector<BindingInfo> _bindingInfos;
};