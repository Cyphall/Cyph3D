#pragma once

#include "Cyph3D/GLObject/GLObject.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

struct TextureCreateInfo;
class GLSampler;

class GLTexture : public GLObject
{
public:
	explicit GLTexture(const TextureCreateInfo& settings);
	~GLTexture() override;
	
	GLuint64 getBindlessTextureHandle() const;
	GLuint64 getBindlessTextureHandle(const GLSampler& sampler) const;
	
	GLuint64 getBindlessImageHandle(GLenum format, GLenum access, int level = 0) const;
	
	void setData(const void* data, GLenum format, GLenum type);
	void setCompressedData(const void* data, GLsizei dataByteSize, glm::ivec2 size, GLint level, GLenum format);
	
	void generateMipmaps();
	
	glm::ivec2 getSize(int level = 0) const;
	
	int getLevels() const;
	
	void clear(GLenum format, GLenum type, void* clearData);

	static int calculateMipmapCount(const glm::ivec2& size);
	
private:
	int _levels;
};