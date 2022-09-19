#include "ExposureEffect.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

ExposureEffect::ExposureEffect(glm::ivec2 size):
PostProcessingEffect("Exposure", size),
_framebuffer(size),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
}),
_shaderProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/exposure/exposure.frag"}
})
{
	_framebuffer.attachColor(0, _outputTexture);
	_framebuffer.addToDrawBuffers(0, 0);
}

GLTexture* ExposureEffect::renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	_shaderProgram.setUniform("u_colorTexture", currentRenderTexture->getBindlessTextureHandle());
	_shaderProgram.setUniform("u_exposure", camera.getExposure());
	
	_framebuffer.bindForDrawing();
	_shaderProgram.bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}