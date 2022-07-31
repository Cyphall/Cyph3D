#include "SkyboxPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Skybox pass"),
_framebuffer(size),
_vao(),
_vbo(36, GL_DYNAMIC_STORAGE_BIT)
{
	_framebuffer.attachColor(0, *textures["gbuffer_color"]);
	_framebuffer.addToDrawBuffers(0, 0);
	
	_framebuffer.attachDepth(*textures["z-prepass_depth"]);
	
	ShaderProgramCreateInfo skyboxShaderProgramCreateInfo;
	skyboxShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/skybox/skybox");
	skyboxShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/skybox/skybox");
	_shader = Engine::getGlobalRM().requestShaderProgram(skyboxShaderProgramCreateInfo);
	
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

void SkyboxPass::preparePipelineImpl()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
}

void SkyboxPass::renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep)
{
	_framebuffer.bindForDrawing();
	
	_shader->bind();
	
	glm::mat4 mvp =
			camera.getProjection() *
			glm::mat4(glm::mat3(camera.getView())) *
			glm::rotate(glm::radians(Engine::getScene().getSkybox()->getRotation()), glm::vec3(0, 1, 0));
	
	_shader->setUniform("u_mvp", mvp);
	
	_shader->setUniform("u_skybox", Engine::getScene().getSkybox()->getResource().getBindlessTextureHandle());
	
	_vao.bind();
	
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SkyboxPass::restorePipelineImpl()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}