#include "Sampler.h"

#include "Cyph3D/GLObject/CreateInfo/SamplerCreateInfo.h"

Sampler::Sampler(const SamplerCreateInfo& settings)
{
	glCreateSamplers(1, &_handle);
	
	glSamplerParameteri(_handle, GL_TEXTURE_MIN_FILTER, settings.minFilter);
	glSamplerParameteri(_handle, GL_TEXTURE_MAG_FILTER, settings.magFilter);
	
	glSamplerParameterf(_handle, GL_TEXTURE_MIN_LOD, settings.minLod);
	glSamplerParameterf(_handle, GL_TEXTURE_MAX_LOD, settings.maxLod);
	
	glSamplerParameterf(_handle, GL_TEXTURE_LOD_BIAS, settings.lodBias);
	
	glSamplerParameteri(_handle, GL_TEXTURE_WRAP_S, settings.wrapS);
	glSamplerParameteri(_handle, GL_TEXTURE_WRAP_T, settings.wrapT);
	glSamplerParameteri(_handle, GL_TEXTURE_WRAP_R, settings.wrapR);
	
	glSamplerParameterfv(_handle, GL_TEXTURE_BORDER_COLOR, settings.borderColor.data());
	
	if (settings.compareFunc != GL_NONE)
	{
		glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(_handle, GL_TEXTURE_COMPARE_FUNC, settings.compareFunc);
	}
	
	if (settings.anisotropicFiltering)
	{
		GLfloat anisoCount;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisoCount);
		glSamplerParameterf(_handle, GL_TEXTURE_MAX_ANISOTROPY, anisoCount);
	}
}

Sampler::~Sampler()
{
	glDeleteSamplers(1, &_handle);
}