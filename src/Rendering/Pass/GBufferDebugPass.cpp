#include "GBufferDebugPass.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

GBufferDebugPass::GBufferDebugPass(std::unordered_map<std::string, Texture*>& textures) :
RenderPass(textures, "GBuffer Debug pass"),
_framebuffer(Engine::getWindow().getSize()),
_debugTexture(TextureCreateInfo
{
	.size = _framebuffer.getSize(),
	.internalFormat = GL_RGBA8
})
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/g-buffer/debug");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/g-buffer/debug");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(createInfo);
	
	_framebuffer.attachColor(0, _debugTexture);
	_framebuffer.addToDrawBuffers(0, 0);
	
	textures["gbuffer_debug"] = &_debugTexture;
}

void GBufferDebugPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void GBufferDebugPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera)
{
	_shader->setUniform("u_viewProjectionInv", glm::inverse(camera.getProjection() * camera.getView()));
	
	_shader->setUniform("u_normalTexture", textures["gbuffer_normal"]);
	_shader->setUniform("u_colorTexture", textures["gbuffer_color"]);
	_shader->setUniform("u_materialTexture", textures["gbuffer_material"]);
	_shader->setUniform("u_geometryNormalTexture", textures["gbuffer_gemoetryNormal"]);
	_shader->setUniform("u_depthTexture", textures["z-prepass_depth"]);
	_shader->setUniform("u_positionTexture", textures["gbuffer_position"]);
	
	_shader->bind();
	_framebuffer.bindForDrawing();
	
	_debugTexture.clear(GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	
	RenderHelper::drawScreenQuad();
}

void GBufferDebugPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}
