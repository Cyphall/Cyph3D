#include "SceneRenderer.h"

#include "Cyph3D/Rendering/Pass/RenderPass.h"

const vk::Format SceneRenderer::DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::HDR_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
const vk::Format SceneRenderer::ACCUMULATION_FORMAT = vk::Format::eR64Uint;
const vk::Format SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format SceneRenderer::FINAL_COLOR_FORMAT = vk::Format::eA2B10G10R10UnormPack32;

SceneRenderer::SceneRenderer(std::string_view name, glm::uvec2 size):
	_size(size)
{
}

glm::uvec2 SceneRenderer::getSize() const
{
	return _size;
}

const VKPtr<VKImage>& SceneRenderer::render(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	const VKPtr<VKImage>& result = onRender(commandBuffer, camera, registry, _firstRender || sceneChanged, _firstRender || cameraChanged);
	_firstRender = false;

	return result;
}

void SceneRenderer::resize(glm::uvec2 size)
{
	_size = size;
	onResize();
}