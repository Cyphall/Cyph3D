#pragma once

#include "PostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"

class ToneMappingEffect : public PostProcessingEffect
{
public:
	ToneMappingEffect();
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
