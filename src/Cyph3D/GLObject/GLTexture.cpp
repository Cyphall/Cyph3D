#include "GLTexture.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/GLSampler.h"

GLTexture::GLTexture(const TextureCreateInfo& settings)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &_handle);
	
	glTextureParameteri(_handle, GL_TEXTURE_MIN_FILTER, settings.minFilter);
	glTextureParameteri(_handle, GL_TEXTURE_MAG_FILTER, settings.magFilter);
	
	glTextureParameterf(_handle, GL_TEXTURE_MIN_LOD, settings.minLod);
	glTextureParameterf(_handle, GL_TEXTURE_MAX_LOD, settings.maxLod);
	
	glTextureParameterf(_handle, GL_TEXTURE_LOD_BIAS, settings.lodBias);
	
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_S, settings.wrapS);
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_T, settings.wrapT);
	glTextureParameteri(_handle, GL_TEXTURE_WRAP_R, settings.wrapR);
	
	glTextureParameterfv(_handle, GL_TEXTURE_BORDER_COLOR, settings.borderColor.data());
	
	glTextureParameteri(_handle, GL_TEXTURE_BASE_LEVEL, settings.baseLevel);
	glTextureParameteri(_handle, GL_TEXTURE_MAX_LEVEL, settings.maxLevel);
	
	glTextureParameteri(_handle, GL_DEPTH_STENCIL_TEXTURE_MODE, settings.depthStencilTextureMode);
	
	if (settings.compareFunc != GL_NONE)
	{
		glTextureParameteri(_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(_handle, GL_TEXTURE_COMPARE_FUNC, settings.compareFunc);
	}
	
	if (settings.anisotropicFiltering)
	{
		GLfloat anisoCount;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisoCount);
		glTextureParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, anisoCount);
		
		_levels = calculateMipmapCount(settings.size);
	}
	else
	{
		_levels = settings.levels;
	}
	
	glTextureParameteriv(_handle, GL_TEXTURE_SWIZZLE_RGBA, settings.swizzle.data());
	
	glTextureStorage2D(_handle, _levels, settings.internalFormat, settings.size.x, settings.size.y);
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &_handle);
	_handle = 0;
}

int GLTexture::calculateMipmapCount(const glm::ivec2& size)
{
	return (int)glm::floor(glm::log2((float)glm::max(size.x, size.y))) + 1;
}

GLuint64 GLTexture::getBindlessTextureHandle() const
{
	GLuint64 bindlessHandle = glGetTextureHandleARB(_handle);
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

GLuint64 GLTexture::getBindlessTextureHandle(const GLSampler& sampler) const
{
	GLuint64 bindlessHandle = glGetTextureSamplerHandleARB(_handle, sampler.getHandle());
	if (!glIsTextureHandleResidentARB(bindlessHandle))
	{
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	return bindlessHandle;
}

void GLTexture::setData(const void* data, GLenum format, GLenum type)
{
	glm::ivec2 size = getSize();
	glTextureSubImage2D(_handle, 0, 0, 0, size.x, size.y, format, type, data);
	generateMipmaps();
}

void GLTexture::setCompressedData(const void* data, GLsizei dataByteSize, glm::ivec2 size, GLint level, GLenum format)
{
	glCompressedTextureSubImage2D(_handle, level, 0, 0, size.x, size.y, format, dataByteSize, data);
}

glm::ivec2 GLTexture::getSize(int level) const
{
	glm::ivec2 size;
	glGetTextureLevelParameteriv(_handle, level, GL_TEXTURE_WIDTH, &size.x);
	glGetTextureLevelParameteriv(_handle, level, GL_TEXTURE_HEIGHT, &size.y);
	return size;
}

int GLTexture::getLevels() const
{
	return _levels;
}

void GLTexture::clear(GLenum format, GLenum type, void* clearData)
{
	glClearTexImage(_handle, 0, format, type, clearData);
}

void GLTexture::generateMipmaps()
{
	if (_levels == 1) return;
	
	glGenerateTextureMipmap(_handle);
}

GLuint64 GLTexture::getBindlessImageHandle(GLenum format, GLenum access, int level) const
{
	GLuint64 bindlessHandle = glGetImageHandleARB(_handle, level, GL_FALSE, 0, format);
	if (!glIsImageHandleResidentARB(bindlessHandle))
	{
		glMakeImageHandleResidentARB(bindlessHandle, access);
	}
	return bindlessHandle;
}