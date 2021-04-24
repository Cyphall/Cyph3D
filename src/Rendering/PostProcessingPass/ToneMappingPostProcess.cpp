#include "ToneMappingPostProcess.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

ToneMappingPostProcess::ToneMappingPostProcess():
_framebuffer(Engine::getWindow().getSize()),
_outputTexture(TextureCreateInfo
{
	.size = _framebuffer.getSize(),
	.internalFormat = GL_RGB8
})
{
	_framebuffer.attachColor(_outputTexture);
	_framebuffer.addToDrawBuffers(_outputTexture, 0);
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/tone mapping/tone mapping");
	_shaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

Texture* ToneMappingPostProcess::render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	_shaderProgram->setUniform("u_colorTexture", currentRenderTexture);
	float exposure = Engine::getScene().getCamera().getExposure();
	_shaderProgram->setUniform("u_exposure", &exposure);
	
	_framebuffer.bindForDrawing();
	_shaderProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}
