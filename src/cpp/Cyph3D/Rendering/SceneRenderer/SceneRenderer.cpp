#include "SceneRenderer.h"

#include <Cyph3D/Rendering/Pass/RenderPass.h>

const vk::Format c3d::SceneRenderer::DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format c3d::SceneRenderer::HDR_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
const vk::Format c3d::SceneRenderer::ACCUMULATION_FORMAT = vk::Format::eR64Uint;
const vk::Format c3d::SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format c3d::SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format c3d::SceneRenderer::FINAL_COLOR_FORMAT = vk::Format::eA2B10G10R10UnormPack32;

c3d::SceneRenderer::SceneRenderer(std::string_view name, glm::uvec2 size):
	_name(name),
	_size(size)
{
}

glm::uvec2 c3d::SceneRenderer::getSize() const
{
	return _size;
}

std::shared_ptr<c3d::VKImage> c3d::SceneRenderer::render(const std::shared_ptr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	commandBuffer->pushDebugGroup(_name);
	std::shared_ptr<VKImage> result = onRender(commandBuffer, camera, registry, _firstRender || sceneChanged, _firstRender || cameraChanged);
	commandBuffer->popDebugGroup();

	_firstRender = false;

	return result;
}

void c3d::SceneRenderer::resize(glm::uvec2 size)
{
	_size = size;
	onResize();
}