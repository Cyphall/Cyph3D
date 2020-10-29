#include "ShaderProgram.h"
#include <stdexcept>
#include <ios>
#include <sstream>
#include "../Helper/ShaderHelper.h"
#include "../Helper/FileHelper.h"
#include "../Helper/StringHelper.h"
#include "../Logger.h"
#include <fmt/core.h>

ShaderProgram::ShaderProgram(const ShaderProgramCreateInfo& createInfo)
{
	_handle = glCreateProgram();
	if (_handle == 0)
	{
		throw std::runtime_error("Unable to create shader program instance");
	}
	
	std::vector<GLuint> shaders;
	
	for (const auto& [type, files] : createInfo.shadersFiles)
	{
		GLuint shader = loadShader(type, files);
		glAttachShader(_handle, shader);
		shaders.push_back(shader);
	}
	
	glLinkProgram(_handle);
	
	for (const GLuint shader : shaders)
	{
		glDetachShader(_handle, shader);
		glDeleteShader(shader);
	}
	
	int linkSuccess;
	glGetProgramiv(_handle, GL_LINK_STATUS, &linkSuccess);
	
	if(linkSuccess == GL_FALSE)
	{
		int length;
		glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &length);
		
		std::string error;
		error.resize(length - 1);
		glGetProgramInfoLog(_handle, length, nullptr, error.data());
		
		throw std::runtime_error(fmt::format("Error while linking shaders to program: {}", error));
	}
	
	int uniformCount;
	glGetProgramInterfaceiv(_handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);
	
	for (int i = 0; i < uniformCount; i++)
	{
		int values[2];
		GLenum properties[] = {GL_NAME_LENGTH, GL_ARRAY_SIZE};
		glGetProgramResourceiv(
				_handle,
				GL_UNIFORM,
				i,
				2,
				properties,
				2,
				nullptr,
				values
		);
		
		std::string name;
		name.resize(values[0] - 1);
		glGetProgramResourceName(_handle, GL_UNIFORM, i, values[0], nullptr, name.data());
		
		if (values[1] > 1)
		{
			remove(name, "[0]");
			_uniforms[name] = glGetUniformLocation(_handle, name.c_str());
			
			for (int j = 0; j < values[1]; j++)
			{
				std::string fullName = fmt::format("{}[{}]", name, j);
				
				_uniforms[fullName] = glGetUniformLocation(_handle, fullName.c_str());
			}
		}
		else
		{
			_uniforms[name] = glGetUniformLocation(_handle, name.c_str());;
		}
	}
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(_handle);
}

void ShaderProgram::bind()
{
	glUseProgram(_handle);
}

GLuint ShaderProgram::loadShader(GLenum type, const std::vector<std::string>& files)
{
	GLuint shader = glCreateShader(type);
	
	if (shader == 0)
	{
		throw std::runtime_error("Unable to create shader instance");
	}
	
	std::string source;
	
	std::string extension = shaderTypeToExtension(type);
	
	try
	{
		source = readAllText(fmt::format("resources/shaders/internal/shaderHeader.{}", extension));
	}
	catch (const std::ios_base::failure& e)
	{
		Logger::Error(fmt::format("Unable to open shader file \"internal/shaderHeader.{}\"", extension));
		throw e;
	}
	
	for (const std::string& file : files)
	{
		try
		{
			source += readAllText(fmt::format("resources/shaders/{}.{}", file, extension));
		}
		catch (const std::ios_base::failure& e)
		{
			Logger::Error(fmt::format("Unable to open shader file \"{}.{}\"", file, extension));
			throw e;
		}
	}
	
	const char* c_source = source.c_str();
	glShaderSource(shader, 1, &c_source, nullptr);
	glCompileShader(shader);
	
	GLint compileSuccess;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	
	if(compileSuccess == GL_FALSE)
	{
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		
		std::string error;
		error.resize(length - 1);
		glGetShaderInfoLog(shader, length, nullptr, error.data());
		
		throw std::runtime_error(fmt::format("Error while compiling shader: {}", error));
	}
	
	return shader;
}

int ShaderProgram::getUniformLocation(const char* name)
{
	try
	{
		return _uniforms.at(name);
	}
	catch (const std::out_of_range& e)
	{
		return -1;
	}
}
