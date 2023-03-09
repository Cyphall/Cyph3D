#pragma once

#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class BloomEffect : public PostProcessingEffect
{
public:
	explicit BloomEffect(glm::ivec2 size);

	GLTexture* renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera) override;

private:
	GLTexture _workTexture;
	GLTexture _outputTexture;

	GLShaderProgram _downsampleProgram;
	GLShaderProgram _upsampleAndBlurProgram;
	GLShaderProgram _composeProgram;

	GLFramebuffer _workFramebuffer;
	GLFramebuffer _composeFramebuffer;

	void copyTextureBaseLevel(GLTexture& source, GLTexture& destination);
	void downsample(GLTexture& texture, int destLevel);
	void upsampleAndBlur(GLTexture& texture, int destLevel, float bloomRadius);
	void compose(GLTexture& sourceA, GLTexture& sourceB, float factor);
};