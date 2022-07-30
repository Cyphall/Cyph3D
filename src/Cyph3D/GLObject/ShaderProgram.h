#pragma once

#include <map>
#include <vector>
#include <glad/glad.h>
#include "BufferBase.h"
#include <glm/glm.hpp>
#include "Texture.h"
#include "Cubemap.h"
#include "CreateInfo/ShaderProgramCreateInfo.h"

class ShaderProgram : public BufferBase
{
public:
	explicit ShaderProgram(const ShaderProgramCreateInfo& createInfo);
	~ShaderProgram() override;
	
	void bind();
	void dispatch(glm::ivec3 groups);
	void dispatchAuto(glm::ivec3 workResolution);
	glm::ivec3 getWorkGroupSize() const;

#pragma region float
	
	void setUniform(const char* name, const float*     data, size_t count = 1);
	void setUniform(const char* name, const glm::vec2* data, size_t count = 1);
	void setUniform(const char* name, const glm::vec3* data, size_t count = 1);
	void setUniform(const char* name, const glm::vec4* data, size_t count = 1);
	
	void setUniform(const char* name, const float&     data);
	void setUniform(const char* name, const glm::vec2& data);
	void setUniform(const char* name, const glm::vec3& data);
	void setUniform(const char* name, const glm::vec4& data);
	
	void setUniform(const char* name, const glm::mat2* data, size_t count = 1);
	void setUniform(const char* name, const glm::mat3* data, size_t count = 1);
	void setUniform(const char* name, const glm::mat4* data, size_t count = 1);
	
	void setUniform(const char* name, const glm::mat2& data);
	void setUniform(const char* name, const glm::mat3& data);
	void setUniform(const char* name, const glm::mat4& data);

#pragma endregion

#pragma region int
	
	void setUniform(const char* name, const int*        data, size_t count = 1);
	void setUniform(const char* name, const glm::ivec2* data, size_t count = 1);
	void setUniform(const char* name, const glm::ivec3* data, size_t count = 1);
	void setUniform(const char* name, const glm::ivec4* data, size_t count = 1);
	
	void setUniform(const char* name, const int&        data);
	void setUniform(const char* name, const glm::ivec2& data);
	void setUniform(const char* name, const glm::ivec3& data);
	void setUniform(const char* name, const glm::ivec4& data);

#pragma endregion

#pragma region uint
	
	void setUniform(const char* name, const uint32_t*   data, size_t count = 1);
	void setUniform(const char* name, const glm::uvec2* data, size_t count = 1);
	void setUniform(const char* name, const glm::uvec3* data, size_t count = 1);
	void setUniform(const char* name, const glm::uvec4* data, size_t count = 1);
	
	void setUniform(const char* name, const uint32_t&   data);
	void setUniform(const char* name, const glm::uvec2& data);
	void setUniform(const char* name, const glm::uvec3& data);
	void setUniform(const char* name, const glm::uvec4& data);
	
#pragma endregion

#pragma region bool
	
	void setUniform(const char* name, const bool*       data, size_t count = 1);
	void setUniform(const char* name, const glm::bvec2* data, size_t count = 1);
	void setUniform(const char* name, const glm::bvec3* data, size_t count = 1);
	void setUniform(const char* name, const glm::bvec4* data, size_t count = 1);
	
	void setUniform(const char* name, const bool&       data);
	void setUniform(const char* name, const glm::bvec2& data);
	void setUniform(const char* name, const glm::bvec3& data);
	void setUniform(const char* name, const glm::bvec4& data);
	
#pragma endregion

#pragma region texture
	
	void setUniform(const char* name, const GLuint64* data, size_t count = 1);
	
	void setUniform(const char* name, const GLuint64& data);

#pragma endregion

private:
	std::map<std::string, int> _uniforms;
	static GLuint loadShader(GLenum type, const std::vector<std::string>& files);
	
	int getUniformLocation(const char* name);
};


