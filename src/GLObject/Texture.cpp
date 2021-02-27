#include "Texture.h"

Texture::Texture(const TextureCreateInfo& settings):
_size(settings.size), _useMipmaps(settings.useMipmaps)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
	
	GLenum minFiltering;
	if (_useMipmaps)
	{
		minFiltering = settings.textureFiltering == NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
	}
	else
	{
		minFiltering = settings.textureFiltering == NEAREST ? GL_NEAREST : GL_LINEAR;
	}
	GLenum magFiltering = settings.textureFiltering == NEAREST ? GL_NEAREST : GL_LINEAR;
	
	glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, minFiltering);
	glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, magFiltering);
	
	if (_useMipmaps)
	{
		GLfloat anisoCount;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisoCount);
		glTextureParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, anisoCount);
	}
	
	if (settings.isShadowMap)
	{
		glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		
		GLfloat borderColor[] = {1, 1, 1, 1};
		glTextureParameterfv(_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
	
	glTextureParameteriv(_handle, GL_TEXTURE_SWIZZLE_RGBA, settings.swizzle.data());
	
	glTextureStorage2D(_handle, _useMipmaps ? calculateMipmapCount(_size) : 1, settings.internalFormat, _size.x, _size.y);
}

Texture::~Texture()
{
	glDeleteTextures(1, &_handle);
}

int Texture::calculateMipmapCount(const glm::ivec2& size)
{
	return (int)glm::floor(glm::log2((float)glm::max(size.x, size.y))) + 1;
}

GLuint64 Texture::getBindlessHandle() const
{
	GLuint64 bindlessHandle = glGetTextureHandleARB(_handle);
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

GLuint64 Texture::getBindlessHandle(const Sampler* sampler) const
{
	GLuint64 bindlessHandle = glGetTextureSamplerHandleARB(_handle, sampler->getHandle());
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

void Texture::setData(const void* data, GLenum format, GLenum type)
{
	glTextureSubImage2D(_handle, 0, 0, 0, _size.x, _size.y, format, type, data);
	if (_useMipmaps)
		glGenerateTextureMipmap(_handle);
}

glm::ivec2 Texture::getSize() const
{
	return _size;
}

void Texture::clear(GLenum format, GLenum type, void* clearData)
{
	glClearTexImage(_handle, 0, format, type, clearData);
}
