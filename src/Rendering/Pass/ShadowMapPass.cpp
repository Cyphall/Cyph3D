#include "ShadowMapPass.h"
#include "../../Window.h"
#include "../../Engine.h"

ShadowMapPass::ShadowMapPass(std::unordered_map<std::string, Texture*>& textures):
RenderPass(textures, "Shadow map pass")
{
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
}

void ShadowMapPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void ShadowMapPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	_vao.bind();
	
	for (DirectionalLight* light : objects.directionalLights)
	{
		light->updateShadowMap(_vao);
	}
	
	for (PointLight* light : objects.pointLights)
	{
		light->updateShadowMap(_vao);
	}
}

void ShadowMapPass::restorePipelineImpl()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glm::ivec2 size = Engine::getWindow().getSize();
	glViewport(0, 0, size.x, size.y);
}
