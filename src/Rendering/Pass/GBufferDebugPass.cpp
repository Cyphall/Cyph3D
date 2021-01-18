#include "GBufferDebugPass.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"
#include "../../Helper/RenderHelper.h"

GBufferDebugPass::GBufferDebugPass(std::unordered_map<std::string, Texture*>& textures) :
IRenderPass(textures),
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
	
	_framebuffer.attachColor(_debugTexture);
	_framebuffer.addToDrawBuffers(_debugTexture, 0);
	
	textures["gbuffer_debug"] = &_debugTexture;
}

void GBufferDebugPass::preparePipeline()
{
	glEnable(GL_CULL_FACE);
}

void GBufferDebugPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	glm::mat4 viewProjectionInv = glm::inverse(camera.getProjection() * camera.getView());
	_shader->setUniform("u_viewProjectionInv", &viewProjectionInv);
	
	Texture* test = textures["gbuffer_normal"];
	_shader->setUniform("u_normalTexture", textures["gbuffer_normal"]);
	_shader->setUniform("u_colorTexture", textures["gbuffer_color"]);
	_shader->setUniform("u_materialTexture", textures["gbuffer_material"]);
	_shader->setUniform("u_geometryNormalTexture", textures["gbuffer_gemoetryNormal"]);
	_shader->setUniform("u_depthTexture", textures["z-prepass_depth"]);
	
	_shader->bind();
	_framebuffer.bindForDrawing();
	
	_debugTexture.clear(GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	
	RenderHelper::drawScreenQuad();
}

void GBufferDebugPass::restorePipeline()
{
	glDisable(GL_CULL_FACE);
}
