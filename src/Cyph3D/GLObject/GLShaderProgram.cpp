#include "GLShaderProgram.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/StringHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/GLObject/GLShader.h"

#include <glm/gtc/type_ptr.hpp>
#include <format>
#include <ios>
#include <numeric>
#include <sstream>
#include <stdexcept>

GLShaderProgram::GLShaderProgram(const std::unordered_map<GLenum, std::string>& shadersPaths)
{
	std::vector<std::unique_ptr<GLShader>> shaders;
	shaders.reserve(shadersPaths.size());
	for (const auto& [type, file] : shadersPaths)
	{
		shaders.push_back(std::make_unique<GLShader>(type, file));
	}
	
	_handle = glCreateProgram();

	for (std::unique_ptr<GLShader>& shader : shaders)
	{
		glAttachShader(_handle, shader->getHandle());
	}
	
	glLinkProgram(_handle);
	
	for (std::unique_ptr<GLShader>& shader : shaders)
	{
		glDetachShader(_handle, shader->getHandle());
	}
	shaders.clear();
	
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
		for (const auto& [shaderType, file] : shadersPaths)
		{
			formattedFiles += std::format("{}\n", file);
		}
		
		this->~GLShaderProgram();
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

int GLShaderProgram::getUniformLocation(const char* name)
{
	auto it = _uniforms.find(name);
	return it != _uniforms.end() ? it->second : -1;
}

void GLShaderProgram::dispatch(glm::uvec3 groups)
{
	glDispatchCompute(groups.x, groups.y, groups.z);
}

void GLShaderProgram::dispatchAuto(glm::uvec3 workResolution)
{
	dispatch(glm::uvec3(glm::ceil(glm::vec3(workResolution) / glm::vec3(getWorkGroupSize()))));
}

glm::uvec3 GLShaderProgram::getWorkGroupSize() const
{
	glm::ivec3 workGroupSize;
	glGetProgramiv(_handle, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(workGroupSize));
	return workGroupSize;
}