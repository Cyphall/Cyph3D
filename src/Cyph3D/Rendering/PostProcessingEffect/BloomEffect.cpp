#include "BloomEffect.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

#include <format>

BloomEffect::BloomEffect(glm::ivec2 size):
PostProcessingEffect("Bloom", size),
_extractBrightFramebuffer(size),
_nonBrightTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
}),
_blurTextures{
	GLTexture(TextureCreateInfo
	{
		.size = size,
		.internalFormat = GL_RGBA16F,
		.minFilter = GL_NEAREST_MIPMAP_NEAREST,
		.magFilter = GL_NEAREST,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1},
		.levels = 6
	}),
	GLTexture(TextureCreateInfo
	{
		.size = size,
		.internalFormat = GL_RGBA16F,
		.minFilter = GL_NEAREST_MIPMAP_NEAREST,
		.magFilter = GL_NEAREST,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1},
		.levels = 6
	})
},
_blurFramebuffers{
	GLFramebuffer(_blurTextures[0].getSize(0)),
	GLFramebuffer(_blurTextures[0].getSize(1)),
	GLFramebuffer(_blurTextures[0].getSize(2)),
	GLFramebuffer(_blurTextures[0].getSize(3)),
	GLFramebuffer(_blurTextures[0].getSize(4)),
	GLFramebuffer(_blurTextures[0].getSize(5))
},
_combineFramebuffer(size),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
}),
_kernelBuffer(GL_DYNAMIC_DRAW),
_extractBrightProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/extract bright.frag"}
}),
_blurProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/blur.frag"}
}),
_passthroughLevelProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/passthroughLevel.frag"}
}),
_combineProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/combine.frag"}
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
}

GLTexture* BloomEffect::renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "extractBright");
	extractBright(currentRenderTexture);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "downsample");
	downsample();
	glPopDebugGroup();
	
	if (_kernelChanged)
		recalculateGaussianKernel();
	
	_kernelBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	
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

void BloomEffect::extractBright(GLTexture* original)
{
	_extractBrightFramebuffer.bindForDrawing();
	
	_extractBrightProgram.setUniform("u_colorTexture", original->getBindlessTextureHandle());
	_extractBrightProgram.bind();
	
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
	
	_blurProgram.setUniform("u_sourceTexture", _blurTextures[0].getBindlessTextureHandle());
	_blurProgram.setUniform("u_horizontal", false);
	_blurProgram.setUniform("u_mipmapLevel", level);
	_blurProgram.bind();
	
	RenderHelper::drawScreenQuad();
	
	
	_blurFramebuffers[level].attachColor(0, _blurTextures[0], level);
	_blurFramebuffers[level].bindForDrawing();
	
	_blurProgram.setUniform("u_sourceTexture", _blurTextures[1].getBindlessTextureHandle());
	_blurProgram.setUniform("u_horizontal", true);
	_blurProgram.setUniform("u_mipmapLevel", level);
	_blurProgram.bind();
	
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
	
	_passthroughLevelProgram.setUniform("u_colorTexture", _blurTextures[0].getBindlessTextureHandle());
	_passthroughLevelProgram.setUniform("u_level", level);
	_passthroughLevelProgram.bind();
	
	RenderHelper::drawScreenQuad();
	
	glm::ivec2 fullSize = _blurTextures[0].getSize(0);
	glViewport(0, 0, fullSize.x, fullSize.y);
	
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
}

void BloomEffect::combine()
{
	_combineFramebuffer.bindForDrawing();
	
	_combineProgram.setUniform("u_colorTexture1", _nonBrightTexture.getBindlessTextureHandle());
	_combineProgram.setUniform("u_colorTexture2", _blurTextures[0].getBindlessTextureHandle());
	_combineProgram.bind();
	
	RenderHelper::drawScreenQuad();
}

float BloomEffect::gaussian(float x, float mu, float sigma)
{
	const float a = ( x - mu ) / sigma;
	return glm::exp( -0.5f * a * a );
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
	std::vector<float> kernel = gaussianKernel(_kernelRadius, _kernelSigma);
	std::span<const float> span(kernel.data() + _kernelRadius, _kernelRadius+1);
	_kernelBuffer.resize(span.size());
	_kernelBuffer.setData(span);
	
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