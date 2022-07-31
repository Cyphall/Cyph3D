#include "ToneMappingEffect.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

ToneMappingEffect::ToneMappingEffect(glm::ivec2 size):
PostProcessingEffect("Tonemapping", size),
_framebuffer(size),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGB8
})
{
	_framebuffer.attachColor(0, _outputTexture);
	_framebuffer.addToDrawBuffers(0, 0);
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/fullscreen quad");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/tone mapping/tone mapping");
	_shaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

Texture* ToneMappingEffect::renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera)
{
	_shaderProgram->setUniform("u_colorTexture", currentRenderTexture->getBindlessTextureHandle());
	
	_framebuffer.bindForDrawing();
	_shaderProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}