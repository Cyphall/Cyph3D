#include "GBufferDebugPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Scene/Camera.h"

GBufferDebugPass::GBufferDebugPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size) :
RenderPass(textures, size, "GBuffer Debug pass"),
_framebuffer(size),
_debugTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA8
}),
_shader({
	{GL_VERTEX_SHADER, "internal/g-buffer/debug.vert"},
	{GL_FRAGMENT_SHADER, "internal/g-buffer/debug.frag"}
})
{
	
	
	_framebuffer.attachColor(0, _debugTexture);
	_framebuffer.addToDrawBuffers(0, 0);
	
	textures["gbuffer_debug"] = &_debugTexture;
}

void GBufferDebugPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void GBufferDebugPass::renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep)
{
	_shader.setUniform("u_viewProjectionInv", glm::inverse(camera.getProjection() * camera.getView()));
	
	_shader.setUniform("u_normalTexture", textures["gbuffer_normal"]->getBindlessTextureHandle());
	_shader.setUniform("u_colorTexture", textures["gbuffer_color"]->getBindlessTextureHandle());
	_shader.setUniform("u_materialTexture", textures["gbuffer_material"]->getBindlessTextureHandle());
	_shader.setUniform("u_geometryNormalTexture", textures["gbuffer_gemoetryNormal"]->getBindlessTextureHandle());
	_shader.setUniform("u_depthTexture", textures["z-prepass_depth"]->getBindlessTextureHandle());
	_shader.setUniform("u_positionTexture", textures["gbuffer_position"]->getBindlessTextureHandle());
	
	_shader.bind();
	_framebuffer.bindForDrawing();
	
	_debugTexture.clear(nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
	
	RenderHelper::drawScreenQuad();
}

void GBufferDebugPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}