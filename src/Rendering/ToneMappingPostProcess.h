#pragma once

#include "IPostProcessEffect.h"
#include "../GLObject/Framebuffer.h"

class ToneMappingPostProcess : public IPostProcessEffect
{
public:
	ToneMappingPostProcess();
	Texture* render(Texture* currentRenderResult, Texture& renderRaw, Texture& depth) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
