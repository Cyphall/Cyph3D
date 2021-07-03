#include <glad/glad.h>
#include "Renderer.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Window.h"
#include "../Scene/Scene.h"
#include "../Engine.h"

Renderer::Renderer(glm::ivec2 size):
_zPrePass(_textures, size),
_shadowMapPass(_textures, size),
_geometryPass(_textures, size),
_gBufferDebugPass(_textures, size),
_skyboxPass(_textures, size),
_lightingPass(_textures, size),
_postProcessingPass(_textures, size),
_objectIndexFramebuffer(size)
{
	_objectIndexFramebuffer.attachColor(0, *_textures["gbuffer_objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

Texture& Renderer::render(Camera& camera, bool debugView)
{
	Scene& scene = Engine::getScene();
	
	render(_zPrePass, camera);
	render(_shadowMapPass, camera);
	
	render(_geometryPass, camera);
	
	if (debugView)
	{
		render(_gBufferDebugPass, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *_textures["gbuffer_debug"];
	}
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isResourceReady())
		render(_skyboxPass, camera);
	
	render(_lightingPass, camera);
	
	render(_postProcessingPass, camera);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return *_textures["final"];
}

void Renderer::onNewFrame()
{
	_registry = RenderRegistry();
}

std::unordered_map<std::string, Texture*>& Renderer::getTextures()
{
	return _textures;
}

RenderRegistry& Renderer::getRegistry()
{
	return _registry;
}

Entity* Renderer::getClickedEntity(glm::ivec2 clickPos)
{
	int objectIndex;
	_objectIndexFramebuffer.bindForReading();
	// shift origin from bottom left to top left
	clickPos.y = _objectIndexFramebuffer.getSize().y - clickPos.y;
	
	glReadPixels(clickPos.x, clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
	
	if (objectIndex != -1)
	{
		return _registry.shapes[objectIndex].owner;
	}
	
	return nullptr;
}

void Renderer::render(RenderPass& pass, Camera& camera)
{
	pass.render(_textures, _registry, camera);
}

void Renderer::requestShapeRendering(ShapeRenderer::RenderData request)
{
	_registry.shapes.push_back(request);
}

void Renderer::requestLightRendering(DirectionalLight::RenderData data)
{
	_registry.directionalLights.push_back(data);
}

void Renderer::requestLightRendering(PointLight::RenderData data)
{
	_registry.pointLights.push_back(data);
}
