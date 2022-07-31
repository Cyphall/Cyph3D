#include "Cubemap.h"

#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include "Cyph3D/GLObject/Sampler.h"

Cubemap::Cubemap(const CubemapCreateInfo& settings):
_size(settings.size)
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_handle);
	
	GLenum minFiltering = GL_LINEAR;
	GLenum magFiltering = settings.textureFiltering;
	
	glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, minFiltering);
	glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, magFiltering);
	
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	glTextureParameteriv(_handle, GL_TEXTURE_SWIZZLE_RGBA, settings.swizzle.data());
	
	
	glTextureStorage2D(_handle, 1, settings.internalFormat, _size.x, _size.y);
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &_handle);
}

GLuint64 Cubemap::getBindlessTextureHandle() const
{
	GLuint64 bindlessHandle = glGetTextureHandleARB(_handle);
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

GLuint64 Cubemap::getBindlessTextureHandle(const Sampler& sampler) const
{
	GLuint64 bindlessHandle = glGetTextureSamplerHandleARB(_handle, sampler.getHandle());
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

void Cubemap::setData(void* data, int face, GLenum format, GLenum type)
{
	glTextureSubImage3D(_handle, 0, 0, 0, face, _size.x, _size.y, 1, format, type, data);
}

void Cubemap::bind(GLuint unit)
{
	glBindTextureUnit(unit, _handle);
}

glm::ivec2 Cubemap::getSize() const
{
	return _size;
}

void Cubemap::clear(GLenum format, GLenum type, void* clearData)
{
	glClearTexImage(_handle, 0, format, type, clearData);
}