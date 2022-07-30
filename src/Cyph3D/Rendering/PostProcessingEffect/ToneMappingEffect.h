#pragma once

#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"
#include "Cyph3D/GLObject/Framebuffer.h"

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