#include "SceneRenderer.h"

#include <Cyph3D/Rendering/Pass/RenderPass.h>

#include <CyphGPU/CommandRecorder.hpp>

const vk::Format c3d::SceneRenderer::DEPTH_FORMAT = vk::Format::eD32Sfloat;
const vk::Format c3d::SceneRenderer::HDR_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
const vk::Format c3d::SceneRenderer::ACCUMULATION_FORMAT = vk::Format::eR32G32B32A32Sfloat;
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

cgpu::ImagePtr c3d::SceneRenderer::render(cgpu::CommandRecorder& commandRecorder, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	sceneChanged |= _firstRender;
	cameraChanged |= _firstRender;
	_firstRender = false;

	cgpu::ScopedDebugRegion debugRegion{commandRecorder, _name};
	return onRender(commandRecorder, camera, registry, sceneChanged, cameraChanged);
}

void c3d::SceneRenderer::resize(glm::uvec2 size)
{
	_size = size;
	onResize();
}
