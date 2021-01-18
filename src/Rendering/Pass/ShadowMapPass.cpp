#include "ShadowMapPass.h"
#include "../../Window.h"
#include "../../Engine.h"

ShadowMapPass::ShadowMapPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures)
{
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
}

void ShadowMapPass::preparePipeline()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void ShadowMapPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
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

void ShadowMapPass::restorePipeline()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glm::ivec2 size = Engine::getWindow().getSize();
	glViewport(0, 0, size.x, size.y);
}
