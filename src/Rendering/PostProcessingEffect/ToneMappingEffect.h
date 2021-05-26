#pragma once

#include "PostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"

class ToneMappingEffect : public PostProcessingEffect
{
public:
	ToneMappingEffect(glm::ivec2 size);
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera) override;

private:
	Framebuffer _framebuffer;
	Texture _outputTexture;
	ShaderProgram* _shaderProgram;
};
