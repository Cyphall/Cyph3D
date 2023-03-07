#pragma once

#include "Cyph3D/GLObject/GLObject.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

struct CubemapCreateInfo;
class GLSampler;

class GLCubemap : public GLObject
{
public:
	explicit GLCubemap(const CubemapCreateInfo& settings);
	~GLCubemap() override;
	
	GLuint64 getBindlessTextureHandle() const;
	GLuint64 getBindlessTextureHandle(const GLSampler& sampler) const;

	GLuint64 getBindlessImageHandle(GLenum format, GLenum access, int level = 0) const;
	
	void setData(const void* data, int face, GLint level, GLenum format, GLenum type);
	void setCompressedData(const void* data, GLsizei dataByteSize, glm::ivec2 size, GLint face, GLint level, GLenum format);

	void generateMipmaps();
	
	glm::ivec2 getSize(int level = 0) const;

	int getLevels() const;
	
	void clear(void* clearData, GLenum format, GLenum type, int level = 0);

	static int calculateMipmapCount(const glm::ivec2& size);

private:
	int _levels;
};