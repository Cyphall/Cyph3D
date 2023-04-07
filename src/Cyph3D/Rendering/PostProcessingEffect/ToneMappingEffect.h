#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class ToneMappingEffect : public PostProcessingEffect
{
public:
	explicit ToneMappingEffect(glm::uvec2 size);
	
	GLTexture& renderImpl(GLTexture& input, Camera& camera) override;

private:
	GLFramebuffer _framebuffer;
	GLTexture _outputTexture;
	GLShaderProgram _shaderProgram;
};