#pragma once

#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKDescriptorSetLayout : public VKObject
{
public:
	static std::shared_ptr<VKDescriptorSetLayout> create(VKContext& context, const VKDescriptorSetLayoutInfo& info);

	~VKDescriptorSetLayout() override;

	const VKDescriptorSetLayoutInfo& getInfo() const;

	const vk::DescriptorSetLayout& getHandle();

private:
	VKDescriptorSetLayout(VKContext& context, const VKDescriptorSetLayoutInfo& info);

	VKDescriptorSetLayoutInfo _info;

	vk::DescriptorSetLayout _handle;
};