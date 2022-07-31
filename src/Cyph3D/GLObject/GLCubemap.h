#pragma once

#include "Cyph3D/GLObject/BufferBase.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

struct CubemapCreateInfo;
class GLSampler;

class GLCubemap : public BufferBase
{
public:
	explicit GLCubemap(const CubemapCreateInfo& settings);
	GLCubemap(const GLCubemap& other) = delete;
	GLCubemap(GLCubemap&& other) = delete;
	
	~GLCubemap() override;
	
	GLuint64 getBindlessTextureHandle() const;
	GLuint64 getBindlessTextureHandle(const GLSampler& sampler) const;
	void setData(void* data, int face, GLenum format, GLenum type);
	void bind(GLuint unit);
	glm::ivec2 getSize() const;
	void clear(GLenum format, GLenum type, void* clearData);

private:
	glm::ivec2 _size;
};