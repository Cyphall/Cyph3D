#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class GLShaderProgram;

class ToneMappingEffect : public PostProcessingEffect
{
public:
	explicit ToneMappingEffect(glm::ivec2 size);
	
	GLTexture* renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera) override;

private:
	GLFramebuffer _framebuffer;
	GLTexture _outputTexture;
	GLShaderProgram* _shaderProgram;
};