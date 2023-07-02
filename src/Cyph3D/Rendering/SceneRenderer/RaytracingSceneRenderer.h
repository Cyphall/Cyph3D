#pragma once

#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/Pass/RaytracePass.h"
#include "Cyph3D/Rendering/Pass/NormalizationPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

class RaytracingSceneRenderer : public SceneRenderer
{
public:
	explicit RaytracingSceneRenderer(glm::uvec2 size);

	Entity* getClickedEntity(glm::uvec2 clickPos) override;
	
	void setSampleCountPerRender(uint32_t count);

private:
	RaytracePass _raytracePass;
	NormalizationPass _normalizationPass;
	ExposurePass _exposurePass;
	BloomPass _bloomPass;
	ToneMappingPass _toneMappingPass;
	
	VKPtr<VKBuffer<int32_t>> _objectIndexBuffer;
	VKPtr<VKImageView> _objectIndexImageView;
	
	uint32_t _sampleCount = 8;
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera) override;
	void onResize() override;
};