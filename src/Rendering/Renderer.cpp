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
	_objectIndexFramebuffer.attachColor(*_textures["gbuffer_objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(*_textures["gbuffer_objectIndex"]);
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
	
	_registry = SceneObjectRegistry();
	for (std::unique_ptr<SceneObject>& object : Engine::getScene().getObjects())
	{
		MeshObject* meshObject = dynamic_cast<MeshObject*>(object.get());
		if (meshObject != nullptr)
		{
			_registry.meshObjects.push_back(meshObject);
		}
		
		DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(object.get());
		if (directionalLight != nullptr)
		{
			_registry.directionalLights.push_back(directionalLight);
		}
		
		PointLight* pointLight = dynamic_cast<PointLight*>(object.get());
		if (pointLight != nullptr)
		{
			_registry.pointLights.push_back(pointLight);
		}
	}
	
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

void Renderer::render(IRenderPass& pass, Camera& camera)
{
	pass.preparePipeline();
	pass.render(_textures, _registry, camera);
	pass.restorePipeline();
}

std::unordered_map<std::string, Texture*>& Renderer::getTextures()
{
	return _textures;
}

SceneObjectRegistry& Renderer::getRegistry()
{
	return _registry;
}

MeshObject* Renderer::getClickedMeshObject(glm::dvec2 clickPos)
{
	int objectIndex;
	_objectIndexFramebuffer.bindForReading();
	// shift origin from bottom left to top left
	clickPos.y = _objectIndexFramebuffer.getSize().y - clickPos.y;
	
	glReadPixels(clickPos.x, clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
	
	if (objectIndex != -1)
	{
		return Engine::getRenderer().getRegistry().meshObjects[objectIndex];
	}
	
	return nullptr;
}
