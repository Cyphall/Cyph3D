#include "VKShader.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/VKObject/VKContext.h"

#include <filesystem>
#include <fstream>
#include <vector>

static std::vector<uint32_t> readSPIRV(const std::filesystem::path& filePath)
{
	size_t size = std::filesystem::file_size(filePath);
	if (size % sizeof(uint32_t) != 0)
	{
		throw;
	}

	std::ifstream file = FileHelper::openFileForReading(filePath);

	std::vector<uint32_t> data(size / sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(data.data()), size);

	return data;
}

VKPtr<VKShader> VKShader::create(VKContext& context, const std::filesystem::path& path)
{
	return VKPtr<VKShader>(new VKShader(context, path));
}

VKShader::VKShader(VKContext& context, const std::filesystem::path& path):
	VKObject(context)
{
	_code = readSPIRV(path);

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

const std::vector<uint32_t>& VKShader::getCode() const
{
	return _code;
}