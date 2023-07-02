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
	
	const VKPtr<VKImageView>& render(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera);
	void resize(glm::uvec2 size);
	
	virtual void onNewFrame();
	
	void requestModelRendering(ModelRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);

	glm::uvec2 getSize() const;
	
	const PerfStep& getRenderPerf();
	
	virtual Entity* getClickedEntity(glm::uvec2 clickPos) = 0;
	
	static const vk::Format DEPTH_FORMAT;
	static const vk::Format HDR_COLOR_FORMAT;
	static const vk::Format ACCUMULATION_FORMAT;
	static const vk::Format OBJECT_INDEX_FORMAT;
	
protected:
	RenderRegistry _registry;
	
	glm::uvec2 _size;

	PerfStep _renderPerf;
	GpuPerfCounter _perfCounter;
	
	virtual const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera) = 0;
	virtual void onResize() = 0;
};