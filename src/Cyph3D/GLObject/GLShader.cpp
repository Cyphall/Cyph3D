#include "GLShader.h"

#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Helper/FileHelper.h"

GLShader::GLShader(GLenum type, const std::string& file)
{
	_handle = glCreateShader(type);

	std::string source;
	try
	{
		source = FileHelper::readAllText(std::format("resources/shaders/{}", file));
	}
	catch (const std::ios_base::failure& e)
	{
		Logger::error(std::format("Unable to open shader file \"{}\"", file));
		throw e;
	}

	const char* c_source = source.c_str();
	glShaderSource(_handle, 1, &c_source, nullptr);
	glCompileShader(_handle);

	GLint compileSuccess;
	glGetShaderiv(_handle, GL_COMPILE_STATUS, &compileSuccess);

	if(compileSuccess == GL_FALSE)
	{
		int length;
		glGetShaderiv(_handle, GL_INFO_LOG_LENGTH, &length);

		std::string error;
		error.resize(length - 1);
		glGetShaderInfoLog(_handle, length, nullptr, error.data());

		this->~GLShader();
		throw std::runtime_error(std::format("Error while compiling shaders:\n{}\n{}", file, error));
	}
}

GLShader::~GLShader()
{
	glDeleteShader(_handle);
	_handle = 0;
}