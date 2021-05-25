#pragma once

#include "PostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"

class ExposureEffect : public PostProcessingEffect
{
public:
	ExposureEffect(glm::ivec2 size);
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
