#include <format>
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
_blurTextures{
	Texture(TextureCreateInfo
	{
		.size = Engine::getWindow().getSize(),
		.internalFormat = GL_RGB16F,
		.minFilter = GL_NEAREST_MIPMAP_NEAREST,
		.magFilter = GL_NEAREST,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1},
		.levels = 6
	}),
	Texture(TextureCreateInfo
	{
		.size = Engine::getWindow().getSize(),
		.internalFormat = GL_RGB16F,
		.minFilter = GL_NEAREST_MIPMAP_NEAREST,
		.magFilter = GL_NEAREST,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1},
		.levels = 6
	})
},
_blurFramebuffers{
	Framebuffer(_blurTextures[0].getSize(0)),
	Framebuffer(_blurTextures[0].getSize(1)),
	Framebuffer(_blurTextures[0].getSize(2)),
	Framebuffer(_blurTextures[0].getSize(3)),
	Framebuffer(_blurTextures[0].getSize(4)),
	Framebuffer(_blurTextures[0].getSize(5))
},
_combineFramebuffer(Engine::getWindow().getSize()),
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
	
	_blurFramebuffers[0].addToDrawBuffers(0, 0);
	_blurFramebuffers[1].addToDrawBuffers(0, 0);
	_blurFramebuffers[2].addToDrawBuffers(0, 0);
	_blurFramebuffers[3].addToDrawBuffers(0, 0);
	_blurFramebuffers[4].addToDrawBuffers(0, 0);
	_blurFramebuffers[5].addToDrawBuffers(0, 0);
	
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
	
	{
		ShaderProgramCreateInfo createInfo;
		createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
		createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/bloom/passthroughLevel");
		_passthroughLevelProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
	}
}

Texture* BloomEffect::renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "extractBright");
	extractBright(currentRenderTexture);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "downsample");
	downsample();
	glPopDebugGroup();
	
	if (_kernelChanged)
		recalculateGaussianKernel();
	
	_kernelBuffer.bind(2);
	
	for (int i = 5; i > 0; i--)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, std::format("blur({})", i).c_str());
		blur(i);
		glPopDebugGroup();
		
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, std::format("combineWithNextLevel({})", i).c_str());
		combineWithNextLevel(i);
		glPopDebugGroup();
	}
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "blur(0)");
	blur(0);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "combine");
	combine();
	glPopDebugGroup();
	
	return &_outputTexture;
}

void BloomEffect::extractBright(Texture* original)
{
	_extractBrightFramebuffer.bindForDrawing();
	
	_extractBrightProgram->setUniform("u_colorTexture", original);
	_extractBrightProgram->bind();
	
	RenderHelper::drawScreenQuad();
}

void BloomEffect::downsample()
{
	_blurTextures[0].generateMipmaps();
}

void BloomEffect::blur(int level)
{
	glm::ivec2 levelSize = _blurTextures[0].getSize(level);
	glViewport(0, 0, levelSize.x, levelSize.y);
	
	
	_blurFramebuffers[level].attachColor(0, _blurTextures[1], level);
	_blurFramebuffers[level].bindForDrawing();
	
	_blurProgram->setUniform("u_sourceTexture", _blurTextures[0]);
	_blurProgram->setUniform("u_horizontal", false);
	_blurProgram->setUniform("u_mipmapLevel", level);
	_blurProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	
	_blurFramebuffers[level].attachColor(0, _blurTextures[0], level);
	_blurFramebuffers[level].bindForDrawing();
	
	_blurProgram->setUniform("u_sourceTexture", _blurTextures[1]);
	_blurProgram->setUniform("u_horizontal", true);
	_blurProgram->setUniform("u_mipmapLevel", level);
	_blurProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	
	glm::ivec2 fullSize = _blurTextures[0].getSize(0);
	glViewport(0, 0, fullSize.x, fullSize.y);
}

void BloomEffect::combineWithNextLevel(int level)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
	glm::ivec2 levelSize = _blurTextures[0].getSize(level-1);
	glViewport(0, 0, levelSize.x, levelSize.y);
	
	_blurFramebuffers[level-1].attachColor(0, _blurTextures[0], level-1);
	_blurFramebuffers[level-1].bindForDrawing();
	
	_passthroughLevelProgram->setUniform("u_colorTexture", _blurTextures[0]);
	_passthroughLevelProgram->setUniform("u_level", level);
	_passthroughLevelProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	glm::ivec2 fullSize = _blurTextures[0].getSize(0);
	glViewport(0, 0, fullSize.x, fullSize.y);
	
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
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
