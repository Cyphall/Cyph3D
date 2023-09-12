#include "SceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

const vk::Format SceneRenderer::DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::HDR_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
const vk::Format SceneRenderer::ACCUMULATION_FORMAT = vk::Format::eR64Uint;
const vk::Format SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;

SceneRenderer::SceneRenderer(std::string_view name, glm::uvec2 size):
	_size(size), _renderPerf(name)
{

}

glm::uvec2 SceneRenderer::getSize() const
{
	return _size;
}

const PerfStep& SceneRenderer::getRenderPerf()
{
	return _renderPerf;
}

const VKPtr<VKImageView>& SceneRenderer::render(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	_renderPerf.clear();
	_renderPerf.setDuration(_perfCounter.retrieve());
	
	_perfCounter.start(commandBuffer);
	
	const VKPtr<VKImageView>& result = onRender(commandBuffer, camera, registry, _firstRender || sceneChanged, _firstRender || cameraChanged);
	_firstRender = false;
	
	_perfCounter.stop(commandBuffer);
	
	return result;
}

void SceneRenderer::resize(glm::uvec2 size)
{
	_size = size;
	onResize();
}