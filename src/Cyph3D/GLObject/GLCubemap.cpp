#include "GLCubemap.h"

#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include "Cyph3D/GLObject/GLSampler.h"

GLCubemap::GLCubemap(const CubemapCreateInfo& settings):
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

GLCubemap::~GLCubemap()
{
	glDeleteTextures(1, &_handle);
	_handle = 0;
}

GLuint64 GLCubemap::getBindlessTextureHandle() const
{
	GLuint64 bindlessHandle = glGetTextureHandleARB(_handle);
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

GLuint64 GLCubemap::getBindlessTextureHandle(const GLSampler& sampler) const
{
	GLuint64 bindlessHandle = glGetTextureSamplerHandleARB(_handle, sampler.getHandle());
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

void GLCubemap::setData(void* data, int face, GLenum format, GLenum type)
{
	glTextureSubImage3D(_handle, 0, 0, 0, face, _size.x, _size.y, 1, format, type, data);
}

void GLCubemap::setCompressedData(const void* data, GLsizei dataByteSize, glm::ivec2 size, GLint face, GLenum format)
{
	glCompressedTextureSubImage3D(_handle, 0, 0, 0, face, size.x, size.y, 1, format, dataByteSize, data);
}

void GLCubemap::bind(GLuint unit)
{
	glBindTextureUnit(unit, _handle);
}

glm::ivec2 GLCubemap::getSize() const
{
	return _size;
}

void GLCubemap::clear(GLenum format, GLenum type, void* clearData)
{
	glClearTexImage(_handle, 0, format, type, clearData);
}