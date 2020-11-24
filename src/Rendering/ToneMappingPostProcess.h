#pragma once

#include "IPostProcessEffect.h"
#include "../GLObject/Framebuffer.h"

class ToneMappingPostProcess : public IPostProcessEffect
{
public:
	ToneMappingPostProcess();
	
	Texture* render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
