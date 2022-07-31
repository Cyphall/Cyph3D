#include "Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/Scene/Camera.h"

ExposureEffect::ExposureEffect(glm::ivec2 size):
PostProcessingEffect("Exposure", size),
_framebuffer(size),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
})
{
	_framebuffer.attachColor(0, _outputTexture);
	_framebuffer.addToDrawBuffers(0, 0);
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/fullscreen quad");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/post-processing/exposure/exposure");
	_shaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

Texture* ExposureEffect::renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera)
{
	_shaderProgram->setUniform("u_colorTexture", currentRenderTexture->getBindlessTextureHandle());
	_shaderProgram->setUniform("u_exposure", camera.getExposure());
	
	_framebuffer.bindForDrawing();
	_shaderProgram->bind();
	
	RenderHelper::drawScreenQuad();
	
	return &_outputTexture;
}