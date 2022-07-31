#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Cyph3D/GLObject/BufferBase.h"

struct CubemapCreateInfo;
class Sampler;

class Cubemap : public BufferBase
{
public:
	explicit Cubemap(const CubemapCreateInfo& settings);
	Cubemap(const Cubemap& other) = delete;
	Cubemap(Cubemap&& other) = delete;
	
	~Cubemap() override;
	
	GLuint64 getBindlessTextureHandle() const;
	GLuint64 getBindlessTextureHandle(const Sampler& sampler) const;
	void setData(void* data, int face, GLenum format, GLenum type);
	void bind(GLuint unit);
	glm::ivec2 getSize() const;
	void clear(GLenum format, GLenum type, void* clearData);

private:
	glm::ivec2 _size;
};