#include "ToneMappingEffect.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Scene/Scene.h"

ToneMappingEffect::ToneMappingEffect(glm::ivec2 size):
PostProcessingEffect("Tonemapping", size),
_framebuffer(size),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGB8
}),
_shaderProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/tone mapping/tone mapping.frag"}
})
{
	_framebuffer.attachColor(0, _outputTexture);
	_framebuffer.addToDrawBuffers(0, 0);
}

GLTexture* ToneMappingEffect::renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	_shaderProgram.setUniform("u_colorTexture", currentRenderTexture->getBindlessTextureHandle());
	
	_framebuffer.bindForDrawing();
	_shaderProgram.bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}