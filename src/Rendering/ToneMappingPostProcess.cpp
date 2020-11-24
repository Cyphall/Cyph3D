#include "ToneMappingPostProcess.h"
#include "../Window.h"
#include "../Scene/Scene.h"
#include "../Engine.h"
#include "../Helper/RenderHelper.h"

ToneMappingPostProcess::ToneMappingPostProcess():
_framebuffer(Engine::getWindow().getSize()),
_outputTexture(TextureCreateInfo
{
	.size = _framebuffer.getSize(),
	.internalFormat = GL_RGB8
})
{
	_framebuffer.attach(GL_COLOR_ATTACHMENT0, _outputTexture);
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("postProcessing/toneMapping");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("postProcessing/toneMapping");
	_shaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

Texture* ToneMappingPostProcess::render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	_shaderProgram->setUniform("colorTexture", currentRenderTexture);
	float exposure = Engine::getScene().getCamera().exposure;
	_shaderProgram->setUniform("exposure", &exposure);
	
	_framebuffer.bind();
	_shaderProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}
