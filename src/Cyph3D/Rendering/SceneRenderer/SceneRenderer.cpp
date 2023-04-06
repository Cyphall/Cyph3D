#include "SceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Scene/Scene.h"

SceneRenderer::SceneRenderer(const char* name, glm::ivec2 size):
_name(name), _size(size)
{

}

void SceneRenderer::render(RenderPass& pass, Camera& camera)
{
	_renderPerf.subSteps.push_back(pass.render(_textures, _registry, camera));
}

void SceneRenderer::onNewFrame()
{
	_registry.clear();
	_renderPerf = PerfStep();
}

void SceneRenderer::requestShapeRendering(ShapeRenderer::RenderData request)
{
	_registry.shapes.push_back(request);
}

void SceneRenderer::requestLightRendering(DirectionalLight::RenderData data)
{
	_registry.directionalLights.push_back(data);
}

void SceneRenderer::requestLightRendering(PointLight::RenderData data)
{
	_registry.pointLights.push_back(data);
}

glm::ivec2 SceneRenderer::getSize() const
{
	return _size;
}

std::pair<GLTexture*, const PerfStep*> SceneRenderer::render(Camera& camera)
{
	Scene& scene = Engine::getScene();
	
	_renderPerf.name = _name;
	_renderPerf.durationInMs = _perfCounter.retrieve();
	
	_perfCounter.start();
	
	GLTexture& result = renderImpl(camera, scene);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	_perfCounter.stop();
	
	return std::make_pair(&result, &_renderPerf);
}