#include <glad/glad.h>
#include "Renderer.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Window.h"
#include "../Scene/Scene.h"
#include "../Engine.h"

Renderer::Renderer():
_shadowMapPass(_textures),
_geometryPass(_textures),
_skyboxPass(_textures),
_lightingPass(_textures),
_postProcessingPass(_textures)
{}

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
	
	SceneObjectRegistry registry;
	for (std::unique_ptr<SceneObject>& object : Engine::getScene().getObjects())
	{
		MeshObject* meshObject = dynamic_cast<MeshObject*>(object.get());
		if (meshObject != nullptr)
		{
			registry.meshObjects.push_back(meshObject);
		}
		
		DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(object.get());
		if (directionalLight != nullptr)
		{
			registry.directionalLights.push_back(directionalLight);
		}
		
		PointLight* pointLight = dynamic_cast<PointLight*>(object.get());
		if (pointLight != nullptr)
		{
			registry.pointLights.push_back(pointLight);
		}
	}
	
	render(_shadowMapPass, registry, camera);
	
	render(_geometryPass, registry, camera);
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isResourceReady())
		render(_skyboxPass, registry, camera);
	
	render(_lightingPass, registry, camera);
	
	if (_debug)
		Framebuffer::drawToDefault(*_textures["raw_render"], true);
	
	render(_postProcessingPass, registry, camera);
	
	Framebuffer::drawToDefault(*_textures["final"], true);
	
}

void Renderer::render(IRenderPass& pass, SceneObjectRegistry& registry, Camera& camera)
{
	pass.preparePipeline();
	pass.render(_textures, registry, camera);
	pass.restorePipeline();
}
