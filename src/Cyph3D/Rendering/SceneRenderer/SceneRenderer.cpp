#include "SceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Scene/Scene.h"

SceneRenderer::SceneRenderer(std::string_view name, glm::uvec2 size):
_name(name), _size(size), _renderPerf(name)
{

}

void SceneRenderer::render(RenderPass& pass, Camera& camera)
{
	_renderPerf.addSubstep(pass.render(_textures, _registry, camera));
}

void SceneRenderer::onNewFrame()
{
	_registry.clear();
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

glm::uvec2 SceneRenderer::getSize() const
{
	return _size;
}

const PerfStep& SceneRenderer::getRenderPerf()
{
	return _renderPerf;
}

GLTexture& SceneRenderer::render(Camera& camera)
{
	Scene& scene = Engine::getScene();
	
	_renderPerf.clear();
	_renderPerf.setDuration(_perfCounter.retrieve());
	
	_perfCounter.start();
	
	GLTexture& result = renderImpl(camera, scene);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	_perfCounter.stop();
	
	return result;
}