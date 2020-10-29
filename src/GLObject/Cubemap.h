#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "CreateInfo/CubemapCreateInfo.h"
#include "BufferBase.h"

class Cubemap : public BufferBase
{
public:
	explicit Cubemap(const CubemapCreateInfo& settings);
	Cubemap(const Cubemap& other) = delete;
	Cubemap(Cubemap&& other) = delete;
	
	~Cubemap() override;
	
	GLuint64 getBindlessHandle() const;
	void setData(void* data, int face, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE);
	void bind(GLuint unit);
	glm::ivec2 getSize() const;

private:
	glm::ivec2 _size;
	GLuint64 _bindlessHandle;
};
