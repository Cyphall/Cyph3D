#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma region float

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

void ShaderProgram::setUniform(const char* name, const float& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::vec2& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::vec3& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::vec4& data)
{
	setUniform(name, &data);
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

void ShaderProgram::setUniform(const char* name, const glm::mat2& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::mat3& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::mat4& data)
{
	setUniform(name, &data);
}

#pragma endregion

#pragma region int

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

void ShaderProgram::setUniform(const char* name, const int& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::ivec2& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::ivec3& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::ivec4& data)
{
	setUniform(name, &data);
}

#pragma endregion

#pragma region uint

void ShaderProgram::setUniform(const char* name, const uint32_t* data, size_t count)
{
	glProgramUniform1uiv(_handle, getUniformLocation(name), count, data);
}
void ShaderProgram::setUniform(const char* name, const glm::uvec2* data, size_t count)
{
	glProgramUniform2uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::uvec3* data, size_t count)
{
	glProgramUniform3uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}
void ShaderProgram::setUniform(const char* name, const glm::uvec4* data, size_t count)
{
	glProgramUniform4uiv(_handle, getUniformLocation(name), count, glm::value_ptr(*data));
}

void ShaderProgram::setUniform(const char* name, const uint32_t& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::uvec2& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::uvec3& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::uvec4& data)
{
	setUniform(name, &data);
}

#pragma endregion

#pragma region bool

void ShaderProgram::setUniform(const char* name, const bool* data, size_t count)
{
	std::vector<GLint> convertedData(count);
	for (int i = 0; i < count; i++)
	{
		convertedData[i] = data[i];
	}
	glProgramUniform1iv(_handle, getUniformLocation(name), count, convertedData.data());
}
void ShaderProgram::setUniform(const char* name, const glm::bvec2* data, size_t count)
{
	std::vector<glm::ivec2> convertedData(count);
	for (int i = 0; i < count; i++)
	{
		convertedData[i] = data[i];
	}
	glProgramUniform2iv(_handle, getUniformLocation(name), count, glm::value_ptr(*convertedData.data()));
}
void ShaderProgram::setUniform(const char* name, const glm::bvec3* data, size_t count)
{
	std::vector<glm::ivec3> convertedData(count);
	for (int i = 0; i < count; i++)
	{
		convertedData[i] = data[i];
	}
	glProgramUniform3iv(_handle, getUniformLocation(name), count, glm::value_ptr(*convertedData.data()));
}
void ShaderProgram::setUniform(const char* name, const glm::bvec4* data, size_t count)
{
	std::vector<glm::ivec4> convertedData(count);
	for (int i = 0; i < count; i++)
	{
		convertedData[i] = data[i];
	}
	glProgramUniform4iv(_handle, getUniformLocation(name), count, glm::value_ptr(*convertedData.data()));
}

void ShaderProgram::setUniform(const char* name, const bool& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec2& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec3& data)
{
	setUniform(name, &data);
}
void ShaderProgram::setUniform(const char* name, const glm::bvec4& data)
{
	setUniform(name, &data);
}

#pragma endregion

#pragma region texture

void ShaderProgram::setUniform(const char* name, const GLuint64* data, size_t count)
{
	glProgramUniformHandleui64vARB(_handle, getUniformLocation(name), count, data);
}

void ShaderProgram::setUniform(const char* name, const GLuint64& data)
{
	setUniform(name, &data);
}

#pragma endregion