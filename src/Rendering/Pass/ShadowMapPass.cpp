#include "ShadowMapPass.h"
#include "../../Window.h"
#include "../../Engine.h"

ShadowMapPass::ShadowMapPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures)
{}

void ShadowMapPass::preparePipeline()
{
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void ShadowMapPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	glDisable(GL_CULL_FACE);
	
	for (DirectionalLight* light : objects.directionalLights)
	{
		light->updateShadowMap();
	}
	
	glEnable(GL_CULL_FACE);
	
	for (PointLight* light : objects.pointLights)
	{
		light->updateShadowMap();
	}
}

void ShadowMapPass::restorePipeline()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glm::ivec2 size = Engine::getWindow().getSize();
	glViewport(0, 0, size.x, size.y);
}
