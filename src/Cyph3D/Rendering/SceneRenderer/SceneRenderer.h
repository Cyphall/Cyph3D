#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Rendering/RenderRegistry.h"

#include <glm/glm.hpp>
#include <string>
#include <string_view>

class Scene;
class Camera;
class VKImageView;

class SceneRenderer
{
public:
	SceneRenderer(std::string_view name, glm::uvec2 size);
	virtual ~SceneRenderer() = default;
	
	const VKPtr<VKImageView>& render(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged);
	void resize(glm::uvec2 size);

	glm::uvec2 getSize() const;
	
	const PerfStep& getRenderPerf();
	
	static const vk::Format DEPTH_FORMAT;
	static const vk::Format HDR_COLOR_FORMAT;
	static const vk::Format ACCUMULATION_FORMAT;
	static const vk::Format DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT;
	static const vk::Format POINT_SHADOW_MAP_DEPTH_FORMAT;
	
protected:
	glm::uvec2 _size;

	PerfStep _renderPerf;
	GpuPerfCounter _perfCounter;
	
	bool _firstRender = true;
	
	virtual const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged) = 0;
	virtual void onResize() = 0;
};