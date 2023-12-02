#pragma once

#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/NormalizationPass.h"
#include "Cyph3D/Rendering/Pass/PathTracePass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"

class PathTracingSceneRenderer : public SceneRenderer
{
public:
	explicit PathTracingSceneRenderer(glm::uvec2 size);

	void setSampleCountPerRender(uint32_t count);

private:
	PathTracePass _pathTracePass;
	NormalizationPass _normalizationPass;
	ExposurePass _exposurePass;
	BloomPass _bloomPass;
	ToneMappingPass _toneMappingPass;

	uint32_t _sampleCount = 8;

	const VKPtr<VKImage>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged) override;
	void onResize() override;
};