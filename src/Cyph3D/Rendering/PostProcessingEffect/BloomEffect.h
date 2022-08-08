#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLMutableBuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class GLShaderProgram;

class BloomEffect : public PostProcessingEffect
{
public:
	explicit BloomEffect(glm::ivec2 size);
	
	GLTexture* renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera) override;
	
	int getKernelRadius() const;
	void setKernelRadius(int kernelRadius);
	
	float getKernelSigma() const;
	void setKernelSigma(float kernelSigma);
	
private:
	GLFramebuffer _extractBrightFramebuffer;
	GLShaderProgram* _extractBrightProgram;
	GLTexture _nonBrightTexture;
	
	std::array<GLTexture, 2> _blurTextures;
	std::array<GLFramebuffer, 6> _blurFramebuffers;
	GLShaderProgram* _blurProgram;
	GLShaderProgram* _passthroughLevelProgram;
	
	GLFramebuffer _combineFramebuffer;
	GLShaderProgram* _combineProgram;
	GLTexture _outputTexture;
	
	GLMutableBuffer<float> _kernelBuffer;
	bool _kernelChanged = true;
	int _kernelRadius = 20;
	float _kernelSigma = 5;
	
	void extractBright(GLTexture* original);
	void downsample();
	void blur(int level);
	void combineWithNextLevel(int level);
	void combine();
	
	static float gaussian(float x, float mu, float sigma);
	static std::vector<float> gaussianKernel(int kernelRadius, float sigma);
	void recalculateGaussianKernel();
};