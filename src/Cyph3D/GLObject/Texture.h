#pragma once

#include "Cyph3D/GLObject/BufferBase.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

struct TextureCreateInfo;
class Sampler;

class Texture : public BufferBase
{
public:
	explicit Texture(const TextureCreateInfo& settings);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other) = delete;
	
	~Texture() override;
	
	GLuint64 getBindlessTextureHandle() const;
	GLuint64 getBindlessTextureHandle(const Sampler& sampler) const;
	
	GLuint64 getBindlessImageHandle(GLenum format, GLenum access, int level = 0) const;
	
	void setData(const void* data, GLenum format, GLenum type);
	void generateMipmaps();
	glm::ivec2 getSize(int level = 0) const;
	void clear(GLenum format, GLenum type, void* clearData);
	
private:
	int _levels;
	
	static int calculateMipmapCount(const glm::ivec2& size);
};