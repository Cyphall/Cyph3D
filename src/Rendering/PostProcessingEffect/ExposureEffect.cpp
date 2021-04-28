#include "ExposureEffect.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

ExposureEffect::ExposureEffect():
PostProcessingEffect("Exposure"),
_framebuffer(Engine::getWindow().getSize()),
_outputTexture(TextureCreateInfo
{
	.size = _framebuffer.getSize(),
	.internalFormat = GL_RGB16F
})
{
	_framebuffer.attachColor(_outputTexture);
	_framebuffer.addToDrawBuffers(_outputTexture, 0);
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/post-processing/post-processing");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/exposure/exposure");
	_shaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

Texture* ExposureEffect::renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	_shaderProgram->setUniform("u_colorTexture", currentRenderTexture);
	float exposure = Engine::getScene().getCamera().getExposure();
	_shaderProgram->setUniform("u_exposure", &exposure);
	
	_framebuffer.bindForDrawing();
	_shaderProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}