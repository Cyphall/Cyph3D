#include "Texture.h"

Texture::Texture(const TextureCreateInfo& settings):
_size(settings.size), _useMipmaps(settings.useMipmaps)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
	
	GLenum minFiltering = _useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	GLenum magFiltering = settings.textureFiltering;
	
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
	
	if (settings.swizzle.has_value())
	{
		glTextureParameteriv(_handle, GL_TEXTURE_SWIZZLE_RGBA, settings.swizzle.value().data());
	}
	
	glTextureStorage2D(_handle, _useMipmaps ? calculateMipmapCount(_size) : 1, settings.internalFormat, _size.x, _size.y);
	
	_bindlessHandle = glGetTextureHandleARB(_handle);
	glMakeTextureHandleResidentARB(_bindlessHandle);
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
	return _bindlessHandle;
}

void Texture::setData(const void* data, GLenum format, GLenum type)
{
	glTextureSubImage2D(_handle, 0, 0, 0, _size.x, _size.y, format, type, data);
	if (_useMipmaps)
		glGenerateTextureMipmap(_handle);
}

void Texture::bind(GLuint unit)
{
	glBindTextureUnit(unit, _handle);
}

glm::ivec2 Texture::getSize() const
{
	return _size;
}
