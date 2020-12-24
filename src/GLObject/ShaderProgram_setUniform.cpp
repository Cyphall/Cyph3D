#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void ShaderProgram::setUniform(const char* name, const float* data, size_t count)
{
	glProgramUniform1fv(_handle, getUniformLocation(name), count, data);
}
void ShaderProgram::setUniform(const char* name, const glm::vec2* data, size_t count)
{
	glProgramUniform2fv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::vec3* data, size_t count)
{
	glProgramUniform3fv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::vec4* data, size_t count)
{
	glProgramUniform4fv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}

void ShaderProgram::setUniform(const char* name, const int* data, size_t count)
{
	glProgramUniform1iv(_handle, getUniformLocation(name), count, data);
}
void ShaderProgram::setUniform(const char* name, const glm::ivec2* data, size_t count)
{
	glProgramUniform2iv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::ivec3* data, size_t count)
{
	glProgramUniform3iv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::ivec4* data, size_t count)
{
	glProgramUniform4iv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}

void ShaderProgram::setUniform(const char* name, const uint32_t* data, size_t count)
{
	glProgramUniform1uiv(_handle, getUniformLocation(name), count, data);
}
void ShaderProgram::setUniform(const char* name, const glm::uvec2* data, size_t count)
{
	glProgramUniform1uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::uvec3* data, size_t count)
{
	glProgramUniform1uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::uvec4* data, size_t count)
{
	glProgramUniform1uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}

void ShaderProgram::setUniform(const char* name, const bool* data, size_t count)
{
	int value = *data;
	setUniform(name, &value, count);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec2* data, size_t count)
{
	glm::ivec2 value = *data;
	setUniform(name, &value, count);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec3* data, size_t count)
{
	glm::ivec3 value = *data;
	setUniform(name, &value, count);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec4* data, size_t count)
{
	glm::ivec4 value = *data;
	setUniform(name, &value, count);
}

void ShaderProgram::setUniform(const char* name, const glm::mat2* data, size_t count)
{
	glProgramUniformMatrix2fv(_handle, getUniformLocation(name), count, false, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::mat3* data, size_t count)
{
	glProgramUniformMatrix3fv(_handle, getUniformLocation(name), count, false, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::mat4* data, size_t count)
{
	glProgramUniformMatrix4fv(_handle, getUniformLocation(name), count, false, glm::value_ptr(*data));
}

void ShaderProgram::setUniform(const char* name, const Texture* data)
{
	glProgramUniformHandleui64ARB(_handle, getUniformLocation(name), data->getBindlessHandle());
}

void ShaderProgram::setUniform(const char* name, const Cubemap* data)
{
	glProgramUniformHandleui64ARB(_handle, getUniformLocation(name), data->getBindlessHandle());
}
