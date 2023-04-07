#include "SkyboxPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"

#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(glm::uvec2 size):
RenderPass(size, "Skybox pass"),
_vao(),
_vbo(36, GL_DYNAMIC_STORAGE_BIT),
_shader({
	{GL_VERTEX_SHADER, "internal/skybox/skybox.vert"},
	{GL_FRAGMENT_SHADER, "internal/skybox/skybox.frag"}
})
{
	_framebuffer.addToDrawBuffers(0, 0);

	std::vector<SkyboxPass::VertexData> data = {
			{{-1.0f,  1.0f, -1.0f}},
			{{-1.0f, -1.0f, -1.0f}},
			{{ 1.0f, -1.0f, -1.0f}},
			{{ 1.0f, -1.0f, -1.0f}},
			{{ 1.0f,  1.0f, -1.0f}},
			{{-1.0f,  1.0f, -1.0f}},
			
			{{-1.0f, -1.0f,  1.0f}},
			{{-1.0f, -1.0f, -1.0f}},
			{{-1.0f,  1.0f, -1.0f}},
			{{-1.0f,  1.0f, -1.0f}},
			{{-1.0f,  1.0f,  1.0f}},
			{{-1.0f, -1.0f,  1.0f}},
			
			{{ 1.0f, -1.0f, -1.0f}},
			{{ 1.0f, -1.0f,  1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{ 1.0f,  1.0f, -1.0f}},
			{{ 1.0f, -1.0f, -1.0f}},
			
			{{-1.0f, -1.0f,  1.0f}},
			{{-1.0f,  1.0f,  1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{ 1.0f, -1.0f,  1.0f}},
			{{-1.0f, -1.0f,  1.0f}},
			
			{{-1.0f,  1.0f, -1.0f}},
			{{ 1.0f,  1.0f, -1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{ 1.0f,  1.0f,  1.0f}},
			{{-1.0f,  1.0f,  1.0f}},
			{{-1.0f,  1.0f, -1.0f}},
			
			{{-1.0f, -1.0f, -1.0f}},
			{{-1.0f, -1.0f,  1.0f}},
			{{ 1.0f, -1.0f, -1.0f}},
			{{ 1.0f, -1.0f, -1.0f}},
			{{-1.0f, -1.0f,  1.0f}},
			{{ 1.0f, -1.0f,  1.0f}}
	};
	
	_vbo.setData(data);
	
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(SkyboxPass::VertexData, position));
	_vao.bindBufferToSlot(_vbo, 0);
}

SkyboxPassOutput SkyboxPass::renderImpl(SkyboxPassInput& input)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	
	_framebuffer.attachColor(0, input.rawRender);
	_framebuffer.attachDepth(input.depth);
	_framebuffer.bindForDrawing();
	
	_shader.bind();
	
	glm::mat4 mvp =
			input.camera.getProjection() *
			glm::mat4(glm::mat3(input.camera.getView())) *
			glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
	
	_shader.setUniform("u_mvp", mvp);
	
	_shader.setUniform("u_skybox", Engine::getScene().getSkybox()->getCubemap().getBindlessTextureHandle());
	
	_vao.bind();
	
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	
	return SkyboxPassOutput{
	
	};
}