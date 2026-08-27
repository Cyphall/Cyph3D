#pragma once

#include <CyphGPU/fwd.hpp>
#include <glm/glm.hpp>

namespace c3d
{
class Camera;
class RenderRegistry;

class SceneRenderer
{
public:
	SceneRenderer(std::string_view name, glm::uvec2 size);
	virtual ~SceneRenderer() = default;

	cgpu::ImagePtr render(cgpu::CommandRecorder& commandRecorder, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged);
	void resize(glm::uvec2 size);

	glm::uvec2 getSize() const;

	static const vk::Format DEPTH_FORMAT;
	static const vk::Format HDR_COLOR_FORMAT;
	static const vk::Format ACCUMULATION_FORMAT;
	static const vk::Format DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT;
	static const vk::Format POINT_SHADOW_MAP_DEPTH_FORMAT;
	static const vk::Format FINAL_COLOR_FORMAT;

protected:
	std::string _name;
	glm::uvec2 _size;

	bool _firstRender = true;

	virtual cgpu::ImagePtr onRender(cgpu::CommandRecorder& commandRecorder, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged) = 0;
	virtual void onResize() = 0;
};
}
