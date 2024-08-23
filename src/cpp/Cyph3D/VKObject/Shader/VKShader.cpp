#include "VKShader.h"

#include "Cyph3D/VKObject/VKContext.h"

#include <cmrc/cmrc.hpp>
#include <filesystem>
#include <fstream>

CMRC_DECLARE(shaders);

std::shared_ptr<VKShader> VKShader::create(VKContext& context, const std::string& path)
{
	return std::shared_ptr<VKShader>(new VKShader(context, path));
}

VKShader::VKShader(VKContext& context, const std::string& path):
	VKObject(context)
{
	cmrc::file spirvFile = cmrc::shaders::get_filesystem().open(std::format("{}.spv", path));

	if ((spirvFile.end() - spirvFile.begin()) % sizeof(uint32_t) != 0)
	{
		throw;
	}

	_code = {
		reinterpret_cast<const uint32_t*>(spirvFile.begin()),
		reinterpret_cast<const uint32_t*>(spirvFile.end())
	};

	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = _code.size() * sizeof(uint32_t);
	createInfo.pCode = _code.data();

	_handle = _context.getDevice().createShaderModule(createInfo);
}

VKShader::~VKShader()
{
	_context.getDevice().destroyShaderModule(_handle);
}

const vk::ShaderModule& VKShader::getHandle()
{
	return _handle;
}

const std::span<const uint32_t>& VKShader::getCode() const
{
	return _code;
}