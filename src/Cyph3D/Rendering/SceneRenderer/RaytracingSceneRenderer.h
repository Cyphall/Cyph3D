#pragma once

#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/Pass/RaytracePass.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

class RaytracingSceneRenderer : public SceneRenderer
{
public:
	explicit RaytracingSceneRenderer(glm::uvec2 size);

	Entity* getClickedEntity(glm::uvec2 clickPos) override;

private:
	RaytracePass _raytracePass;
	PostProcessingPass _postProcessingPass;
	
	VKPtr<VKBuffer<int32_t>> _objectIndexBuffer;
	VKPtr<VKImageView> _objectIndexImageView;
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera) override;
	void onResize() override;
};