#pragma once

#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>

class VKCommandBuffer;
class Scene;
class Camera;
class VKImage;

class SceneRenderer
{
public:
	SceneRenderer(std::string_view name, glm::uvec2 size);
	virtual ~SceneRenderer() = default;

	const VKPtr<VKImage>& render(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged);
	void resize(glm::uvec2 size);

	glm::uvec2 getSize() const;

	static const vk::Format DEPTH_FORMAT;
	static const vk::Format HDR_COLOR_FORMAT;
	static const vk::Format ACCUMULATION_FORMAT;
	static const vk::Format DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT;
	static const vk::Format POINT_SHADOW_MAP_DEPTH_FORMAT;
	static const vk::Format FINAL_COLOR_FORMAT;

protected:
	glm::uvec2 _size;

	bool _firstRender = true;

	virtual const VKPtr<VKImage>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged) = 0;
	virtual void onResize() = 0;
};