#include "SkyboxPass.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"
#include "../../Scene/Scene.h"
#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(std::unordered_map<std::string, Texture*>& textures):
		IRenderPass(textures),
		_framebuffer(Engine::getWindow().getSize()),
		_vao(),
		_vbo(36, GL_DYNAMIC_STORAGE_BIT)
{
	_framebuffer.attachColor(*textures["gbuffer_color"]);
	_framebuffer.attachDepth(*textures["gbuffer_depth"]);
	
	_framebuffer.addToDrawBuffers(*textures["gbuffer_color"], 0);
	
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

void SkyboxPass::preparePipeline()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
}

void SkyboxPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	_framebuffer.bindForDrawing();
	
	_shader->bind();
	
	glm::mat4 model = glm::rotate(glm::radians(Engine::getScene().getSkybox()->getRotation()), glm::vec3(0, 1, 0));
	_shader->setUniform("model", &model);
	glm::mat4 modifiedView = glm::mat4(glm::mat3(camera.getView()));
	_shader->setUniform("view", &modifiedView);
	glm::mat4 projection = camera.getProjection();
	_shader->setUniform("projection", &projection);
	
	_shader->setUniform("skybox", &Engine::getScene().getSkybox()->getResource());
	
	_vao.bind();
	
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SkyboxPass::restorePipeline()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}
