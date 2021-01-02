#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "CreateInfo/TextureCreateInfo.h"
#include "BufferBase.h"

class Texture : public BufferBase
{
public:
	explicit Texture(const TextureCreateInfo& settings);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other) = delete;
	
	~Texture() override;
	
	GLuint64 getBindlessHandle() const;
	void setData(const void* data, GLenum format, GLenum type);
	void bind(GLuint unit);
	glm::ivec2 getSize() const;
	void clear(GLenum format, GLenum type, void* clearData);
	
private:
	glm::ivec2 _size;
	bool _useMipmaps;
	GLuint64 _bindlessHandle;
	
	static int calculateMipmapCount(const glm::ivec2& size);
};


