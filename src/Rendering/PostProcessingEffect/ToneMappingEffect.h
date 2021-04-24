#pragma once

#include "../IPostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"

class ToneMappingEffect : public IPostProcessingEffect
{
public:
	ToneMappingEffect();
	
	Texture* render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
