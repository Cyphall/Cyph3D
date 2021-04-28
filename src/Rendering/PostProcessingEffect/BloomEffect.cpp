#include "BloomEffect.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

BloomEffect::BloomEffect():
PostProcessingEffect("Bloom"),
_extractBrightFramebuffer(Engine::getWindow().getSize()),
_blurFramebuffer(Engine::getWindow().getSize()),
_combineFramebuffer(Engine::getWindow().getSize()),
_blurTextures{
	Texture(TextureCreateInfo
	{
		.size = _blurFramebuffer.getSize(),
		.internalFormat = GL_RGB16F,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1}
	}),
	Texture(TextureCreateInfo
	{
		.size = _blurFramebuffer.getSize(),
		.internalFormat = GL_RGB16F,
		.wrapS = GL_CLAMP_TO_BORDER,
		.wrapT = GL_CLAMP_TO_BORDER,
		.borderColor = {0, 0, 0, 1}
	})
},
_outputTexture(TextureCreateInfo
{
	.size = _blurFramebuffer.getSize(),
	.internalFormat = GL_RGB16F
})
{
	_extractBrightFramebuffer.attachColor(_blurTextures[0]);
	_extractBrightFramebuffer.addToDrawBuffers(_blurTextures[0], 0);
	
	_blurFramebuffer.attachColor(_blurTextures[0]);
	_blurFramebuffer.attachColor(_blurTextures[1]);
	
	_combineFramebuffer.attachColor(_outputTexture);
	_combineFramebuffer.addToDrawBuffers(_outputTexture, 0);
	
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
	
	blur();
	blur();
	blur();
	blur();
	blur();
	
	combine(currentRenderTexture);
	
	return &_outputTexture;
}

void BloomEffect::extractBright(Texture* original)
{
	_blurFramebuffer.addToDrawBuffers(_blurTextures[0], 0);
	_blurFramebuffer.bindForDrawing();
	
	_extractBrightProgram->setUniform("u_colorTexture", original);
	_extractBrightProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	_blurFramebuffer.removeFromDrawBuffers(_blurTextures[0]);
}

void BloomEffect::blur()
{
	{
		_blurFramebuffer.addToDrawBuffers(_blurTextures[1], 0);
		_blurFramebuffer.bindForDrawing();
		
		_blurProgram->setUniform("u_brightOnlyTexture", &_blurTextures[0]);
		bool temp = false;
		_blurProgram->setUniform("u_horizontal", &temp);
		_blurProgram->bind();
		
		RenderHelper::drawScreenQuad();
		
		_blurFramebuffer.removeFromDrawBuffers(_blurTextures[1]);
	}
	
	{
		_blurFramebuffer.addToDrawBuffers(_blurTextures[0], 0);
		_blurFramebuffer.bindForDrawing();
		
		_blurProgram->setUniform("u_brightOnlyTexture", &_blurTextures[1]);
		bool temp = true;
		_blurProgram->setUniform("u_horizontal", &temp);
		_blurProgram->bind();
		
		RenderHelper::drawScreenQuad();
		
		_blurFramebuffer.removeFromDrawBuffers(_blurTextures[0]);
	}
}

void BloomEffect::combine(Texture* original)
{
	_combineFramebuffer.bindForDrawing();
	
	_combineProgram->setUniform("u_colorTexture1", original);
	_combineProgram->setUniform("u_colorTexture2", &_blurTextures[0]);
	_combineProgram->bind();
	
	RenderHelper::drawScreenQuad();
}
