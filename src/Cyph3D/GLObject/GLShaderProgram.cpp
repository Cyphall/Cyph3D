#include "GLShaderProgram.h"

#include "Cyph3D/GLObject/CreateInfo/ShaderProgramCreateInfo.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/ShaderHelper.h"
#include "Cyph3D/Helper/StringHelper.h"
#include "Cyph3D/Logging/Logger.h"

#include <glm/gtc/type_ptr.hpp>
#include <format>
#include <ios>
#include <numeric>
#include <sstream>
#include <stdexcept>

GLShaderProgram::GLShaderProgram(const ShaderProgramCreateInfo& createInfo)
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
		
		std::string formattedFiles;
		for (const std::pair<GLenum, std::vector<std::string>>& files : createInfo.shadersFiles)
		{
			std::string extension = ShaderHelper::shaderTypeToExtension(files.first);
			for (const std::string& file : files.second)
				formattedFiles += std::format("{}.{}\n", file, extension);
		}
		
		throw std::runtime_error(std::format("Error while linking shaders:\n{}{}", formattedFiles, error));
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
			StringHelper::remove(name, "[0]");
			_uniforms[name] = glGetUniformLocation(_handle, name.c_str());
			
			for (int j = 0; j < values[1]; j++)
			{
				std::string fullName = std::format("{}[{}]", name, j);
				
				_uniforms[fullName] = glGetUniformLocation(_handle, fullName.c_str());
			}
		}
		else
		{
			_uniforms[name] = glGetUniformLocation(_handle, name.c_str());;
		}
	}
}

GLShaderProgram::~GLShaderProgram()
{
	glDeleteProgram(_handle);
	_handle = 0;
}

void GLShaderProgram::bind()
{
	glUseProgram(_handle);
}

GLuint GLShaderProgram::loadShader(GLenum type, const std::vector<std::string>& files)
{
	GLuint shader = glCreateShader(type);
	
	if (shader == 0)
	{
		throw std::runtime_error("Unable to create shader instance");
	}
	
	std::string source;
	
	std::string extension = ShaderHelper::shaderTypeToExtension(type);
	
	for (const std::string& file : files)
	{
		try
		{
			source += FileHelper::readAllText(std::format("resources/shaders/{}.{}", file, extension));
		}
		catch (const std::ios_base::failure& e)
		{
			Logger::error(std::format("Unable to open shader file \"{}.{}\"", file, extension));
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
		
		std::string formattedFiles;
		for (const std::string& file : files)
			formattedFiles += std::format("{}.{}\n", file, extension);
		
		throw std::runtime_error(std::format("Error while compiling shaders:\n{}{}", formattedFiles, error));
	}
	
	return shader;
}

int GLShaderProgram::getUniformLocation(const char* name)
{
	auto it = _uniforms.find(name);
	return it != _uniforms.end() ? it->second : -1;
}

void GLShaderProgram::dispatch(glm::ivec3 groups)
{
	glDispatchCompute(groups.x, groups.y, groups.z);
}

void GLShaderProgram::dispatchAuto(glm::ivec3 workResolution)
{
	dispatch(glm::ivec3(glm::ceil(glm::vec3(workResolution) / glm::vec3(getWorkGroupSize()))));
}

glm::ivec3 GLShaderProgram::getWorkGroupSize() const
{
	glm::ivec3 workGroupSize;
	glGetProgramiv(_handle, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(workGroupSize));
	return workGroupSize;
}