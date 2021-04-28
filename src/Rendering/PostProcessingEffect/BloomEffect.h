#pragma once

#include "PostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"

class BloomEffect : public PostProcessingEffect
{
public:
	BloomEffect();
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;

private:
	Framebuffer _extractBrightFramebuffer;
	ShaderProgram* _extractBrightProgram;
	
	Framebuffer _blurFramebuffer;
	ShaderProgram* _blurProgram;
	
	Framebuffer _combineFramebuffer;
	ShaderProgram* _combineProgram;
	Texture _outputTexture;
	
	std::array<Texture, 2> _blurTextures;
	
	void extractBright(Texture* original);
	void blur();
	void combine(Texture* original);
};
