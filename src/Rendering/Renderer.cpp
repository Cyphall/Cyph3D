#include <glad/glad.h>
#include "Renderer.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Window.h"
#include "../Scene/Scene.h"
#include "../Engine.h"

Renderer::Renderer():
_zPrePass(_textures),
_shadowMapPass(_textures),
_geometryPass(_textures),
_gBufferDebugPass(_textures),
_skyboxPass(_textures),
_lightingPass(_textures),
_postProcessingPass(_textures),
_objectIndexFramebuffer(Engine::getWindow().getSize())
{
	_objectIndexFramebuffer.attachColor(0, *_textures["gbuffer_objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

bool Renderer::getDebug() const
{
	return _debug;
}

void Renderer::setDebug(bool debug)
{
	_debug = debug;
}

void Renderer::render()
{
	Scene& scene = Engine::getScene();
	Camera& camera = scene.getCamera();
	
	render(_zPrePass, camera);
	render(_shadowMapPass, camera);
	
	render(_geometryPass, camera);
	
	if (_debug)
	{
		render(_gBufferDebugPass, camera);
		Framebuffer::drawToDefault(*_textures["gbuffer_debug"], true);
		return;
	}
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isResourceReady())
		render(_skyboxPass, camera);
	
	render(_lightingPass, camera);
	
	render(_postProcessingPass, camera);
	
	Framebuffer::drawToDefault(*_textures["final"], true);
	
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

Entity* Renderer::getClickedEntity(glm::dvec2 clickPos)
{
	int objectIndex;
	_objectIndexFramebuffer.bindForReading();
	// shift origin from bottom left to top left
	clickPos.y = _objectIndexFramebuffer.getSize().y - clickPos.y;
	
	glReadPixels(clickPos.x, clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
	
	if (objectIndex != -1)
	{
		return Engine::getRenderer().getRegistry().meshes[objectIndex].owner;
	}
	
	return nullptr;
}

void Renderer::render(RenderPass& pass, Camera& camera)
{
	pass.render(_textures, _registry, camera);
}

void Renderer::requestMeshRendering(MeshRenderer::RenderData request)
{
	_registry.meshes.push_back(request);
}

void Renderer::requestLightRendering(DirectionalLight::RenderData data)
{
	_registry.directionalLights.push_back(data);
}

void Renderer::requestLightRendering(PointLight::RenderData data)
{
	_registry.pointLights.push_back(data);
}
