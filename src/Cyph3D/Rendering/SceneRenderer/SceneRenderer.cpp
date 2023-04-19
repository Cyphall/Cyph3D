#include "SceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Scene/Scene.h"

const vk::Format SceneRenderer::DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::HDR_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
const vk::Format SceneRenderer::OBJECT_INDEX_FORMAT = vk::Format::eR32Sint;

SceneRenderer::SceneRenderer(std::string_view name, glm::uvec2 size):
	_size(size), _renderPerf(name)
{

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

const VKPtr<VKImageView>& SceneRenderer::render(Camera& camera)
{
	const VKPtr<VKCommandBuffer>& commandBuffer = Engine::getVKContext().getDefaultCommandBuffer();
	
	_renderPerf.clear();
	_renderPerf.setDuration(_perfCounter.retrieve(commandBuffer));
	
	_perfCounter.start(commandBuffer);
	
	const VKPtr<VKImageView>& result = onRender(commandBuffer, camera);
	
	_perfCounter.stop(commandBuffer);
	
	return result;
}

void SceneRenderer::resize(glm::uvec2 size)
{
	_size = size;
	onResize();
}