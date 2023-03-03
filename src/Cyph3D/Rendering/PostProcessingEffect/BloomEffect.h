#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

#include <memory>

class BloomEffect : public PostProcessingEffect
{
public:
	explicit BloomEffect(glm::ivec2 size);
	
	GLTexture* renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera) override;
	
private:
	struct KernelSampleInfo
	{
		float weight;
		float offset;
	};
	
	GLFramebuffer _extractBrightFramebuffer;
	GLShaderProgram _extractBrightProgram;
	GLTexture _nonBrightTexture;
	
	std::array<GLTexture, 2> _blurTextures;
	std::array<GLFramebuffer, 6> _blurFramebuffers;
	GLShaderProgram _blurProgram;
	GLShaderProgram _passthroughLevelProgram;
	
	GLFramebuffer _combineFramebuffer;
	GLShaderProgram _combineProgram;
	GLTexture _outputTexture;
	
	std::unique_ptr<GLImmutableBuffer<KernelSampleInfo>> _kernelBuffer;
	
	void extractBright(GLTexture* original);
	void downsample();
	void blur(int level);
	void combineWithNextLevel(int level);
	void combine();
	
	static float gaussian(float x, float mu, float sigma);
	static std::vector<float> gaussianKernel(int kernelRadius, float sigma);
};