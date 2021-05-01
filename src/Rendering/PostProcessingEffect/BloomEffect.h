#pragma once

#include "PostProcessingEffect.h"
#include "../../GLObject/Framebuffer.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class BloomEffect : public PostProcessingEffect
{
public:
	BloomEffect();
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) override;
	
	int getKernelRadius() const;
	void setKernelRadius(int kernelRadius);
	
	float getKernelSigma() const;
	void setKernelSigma(float kernelSigma);
	
private:
	Framebuffer _extractBrightFramebuffer;
	ShaderProgram* _extractBrightProgram;
	Texture _nonBrightTexture;
	
	Framebuffer _blurFramebuffer;
	ShaderProgram* _blurProgram;
	
	Framebuffer _combineFramebuffer;
	ShaderProgram* _combineProgram;
	Texture _outputTexture;
	
	std::array<Texture, 2> _blurTextures;
	
	ShaderStorageBuffer<float> _kernelBuffer;
	bool _kernelChanged = true;
	int _kernelRadius = 20;
	float _kernelSigma = 5;
	
	void extractBright(Texture* original);
	void blur();
	void combine();
	
	float gaussian(float x, float mu, float sigma);
	std::vector<float> gaussianKernel(int kernelRadius, float sigma);
	void recalculateGaussianKernel();
};
