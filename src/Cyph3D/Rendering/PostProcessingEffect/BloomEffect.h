#pragma once

#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"
#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/GLObject/ShaderStorageBuffer.h"
#include "Cyph3D/GLObject/Texture.h"

class ShaderProgram;

class BloomEffect : public PostProcessingEffect
{
public:
	BloomEffect(glm::ivec2 size);
	
	Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera) override;
	
	int getKernelRadius() const;
	void setKernelRadius(int kernelRadius);
	
	float getKernelSigma() const;
	void setKernelSigma(float kernelSigma);
	
private:
	Framebuffer _extractBrightFramebuffer;
	ShaderProgram* _extractBrightProgram;
	Texture _nonBrightTexture;
	
	std::array<Texture, 2> _blurTextures;
	std::array<Framebuffer, 6> _blurFramebuffers;
	ShaderProgram* _blurProgram;
	ShaderProgram* _passthroughLevelProgram;
	
	Framebuffer _combineFramebuffer;
	ShaderProgram* _combineProgram;
	Texture _outputTexture;
	
	ShaderStorageBuffer<float> _kernelBuffer;
	bool _kernelChanged = true;
	int _kernelRadius = 20;
	float _kernelSigma = 5;
	
	void extractBright(Texture* original);
	void downsample();
	void blur(int level);
	void combineWithNextLevel(int level);
	void combine();
	
	float gaussian(float x, float mu, float sigma);
	std::vector<float> gaussianKernel(int kernelRadius, float sigma);
	void recalculateGaussianKernel();
};