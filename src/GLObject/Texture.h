#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <map>
#include "CreateInfo/TextureCreateInfo.h"
#include "BufferBase.h"
#include "Sampler.h"

class Texture : public BufferBase
{
public:
	explicit Texture(const TextureCreateInfo& settings);
	Texture(const Texture& other) = delete;
	Texture(Texture&& other) = delete;
	
	~Texture() override;
	
	GLuint64 getBindlessHandle() const;
	GLuint64 getBindlessHandle(const Sampler* sampler) const;
	void setData(const void* data, GLenum format, GLenum type);
	glm::ivec2 getSize() const;
	void clear(GLenum format, GLenum type, void* clearData);
	
private:
	glm::ivec2 _size;
	bool _anisotropicFiltering;
	
	static int calculateMipmapCount(const glm::ivec2& size);
};


