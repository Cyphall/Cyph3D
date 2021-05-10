#include "BloomEffect.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

BloomEffect::BloomEffect():
PostProcessingEffect("Bloom"),
_extractBrightFramebuffer(Engine::getWindow().getSize()),
_nonBrightTexture(TextureCreateInfo
{
	.size = Engine::getWindow().getSize(),
	.internalFormat = GL_RGB16F
}),
_blurFramebuffer(Engine::getWindow().getSize()),
_combineFramebuffer(Engine::getWindow().getSize()),
_blurTextures{
	Texture(TextureCreateInfo
	{
		.size = Engine::getWindow().getSize(),
		.internalFormat = GL_RGB16F,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1}
	}),
	Texture(TextureCreateInfo
	{
		.size = Engine::getWindow().getSize(),
		.internalFormat = GL_RGB16F,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1}
	})
},
_outputTexture(TextureCreateInfo
{
	.size = Engine::getWindow().getSize(),
	.internalFormat = GL_RGB16F
})
{
	_extractBrightFramebuffer.attachColor(0, _nonBrightTexture);
	_extractBrightFramebuffer.attachColor(1, _blurTextures[0]);
	_extractBrightFramebuffer.addToDrawBuffers(0, 0);
	_extractBrightFramebuffer.addToDrawBuffers(1, 1);
	
	_blurFramebuffer.addToDrawBuffers(0, 0);
	
	_combineFramebuffer.attachColor(0, _outputTexture);
	_combineFramebuffer.addToDrawBuffers(0, 0);
	
	{
		ShaderProgramCreateInfo createInfo;
		createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
		createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/bloom/extract bright");
		_extractBrightProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
	}
	
	{
		ShaderProgramCreateInfo createInfo;
		createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
		createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/bloom/blur");
		_blurProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
	}
	
	{
		ShaderProgramCreateInfo createInfo;
		createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
		createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/bloom/combine");
		_combineProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
	}
}

Texture* BloomEffect::renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	extractBright(currentRenderTexture);
	
	if (_kernelChanged)
		recalculateGaussianKernel();
	
	_kernelBuffer.bind(2);
	
	blur();
	blur();
	blur();
	blur();
	blur();
	
	combine();
	
	return &_outputTexture;
}

void BloomEffect::extractBright(Texture* original)
{
	_extractBrightFramebuffer.bindForDrawing();
	
	_extractBrightProgram->setUniform("u_colorTexture", original);
	_extractBrightProgram->bind();
	
	RenderHelper::drawScreenQuad();
}

void BloomEffect::blur()
{
	_blurFramebuffer.attachColor(0, _blurTextures[1]);
	_blurFramebuffer.bindForDrawing();
	
	_blurProgram->setUniform("u_brightOnlyTexture", _blurTextures[0]);
	_blurProgram->setUniform("u_horizontal", false);
	_blurProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	
	_blurFramebuffer.attachColor(0, _blurTextures[0]);
	_blurFramebuffer.bindForDrawing();
	
	_blurProgram->setUniform("u_brightOnlyTexture", _blurTextures[1]);
	_blurProgram->setUniform("u_horizontal", true);
	_blurProgram->bind();
	
	RenderHelper::drawScreenQuad();
}

void BloomEffect::combine()
{
	_combineFramebuffer.bindForDrawing();
	
	_combineProgram->setUniform("u_colorTexture1", _nonBrightTexture);
	_combineProgram->setUniform("u_colorTexture2", _blurTextures[0]);
	_combineProgram->bind();
	
	RenderHelper::drawScreenQuad();
}

float BloomEffect::gaussian(float x, float mu, float sigma)
{
	const float a = ( x - mu ) / sigma;
	return glm::exp( -0.5 * a * a );
}

std::vector<float> BloomEffect::gaussianKernel(int kernelRadius, float sigma)
{
	std::vector<float> kernel(2*kernelRadius+1);
	
	float sum = 0;
	// compute values
	for (int i = 0; i < kernel.size(); i++)
	{
		float x = gaussian(i, kernelRadius, sigma);
		kernel[i] = x;
		sum += x;
	}
	
	// normalize
	for (float& value : kernel)
		value /= sum;
	return kernel;
}

void BloomEffect::recalculateGaussianKernel()
{
	_kernelBuffer.setData(gaussianKernel(_kernelRadius, _kernelSigma).data() + _kernelRadius, _kernelRadius+1);
	
	_kernelChanged = false;
}

int BloomEffect::getKernelRadius() const
{
	return _kernelRadius;
}

void BloomEffect::setKernelRadius(int kernelRadius)
{
	if (kernelRadius == _kernelRadius) return;
	
	_kernelRadius = kernelRadius;
	_kernelChanged = true;
}

float BloomEffect::getKernelSigma() const
{
	return _kernelSigma;
}

void BloomEffect::setKernelSigma(float kernelSigma)
{
	if (kernelSigma == _kernelSigma) return;
	
	_kernelSigma = kernelSigma;
	_kernelChanged = true;
}
