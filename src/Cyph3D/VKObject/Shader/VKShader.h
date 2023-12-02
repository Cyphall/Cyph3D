#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>
#include <vulkan/vulkan.hpp>

class VKShader : public VKObject
{
public:
	static VKPtr<VKShader> create(VKContext& context, const std::filesystem::path& path);

	~VKShader() override;

	const vk::ShaderModule& getHandle();
	const std::vector<uint32_t>& getCode() const;

private:
	VKShader(VKContext& context, const std::filesystem::path& path);

	vk::ShaderModule _handle;
	std::vector<uint32_t> _code;
};