#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <string>
#include <vulkan/vulkan.hpp>

class VKShader : public VKObject
{
public:
	static std::shared_ptr<VKShader> create(VKContext& context, const std::string& path);

	~VKShader() override;

	const vk::ShaderModule& getHandle();
	const std::span<const uint32_t>& getCode() const;

private:
	VKShader(VKContext& context, const std::string& path);

	vk::ShaderModule _handle;
	std::span<const uint32_t> _code;
};