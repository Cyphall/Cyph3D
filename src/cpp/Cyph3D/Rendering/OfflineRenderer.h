#pragma once

#include <Cyph3D/Rendering/Pass/BloomPass.h>
#include <Cyph3D/Rendering/Pass/ExposurePass.h>
#include <Cyph3D/Rendering/Pass/NormalizationPass.h>
#include <Cyph3D/Rendering/Pass/PathTracePass.h>
#include <Cyph3D/Rendering/Pass/ToneMappingPass.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <glm/glm.hpp>

namespace c3d
{
class OfflineRenderer
{
public:
	explicit OfflineRenderer(glm::uvec2 size, Camera& camera, const RenderRegistry& registry);

	void traceRays(cgpu::CommandRecorder& cmdRec, uint32_t rayCount);

	cgpu::ImagePtr postProcess(cgpu::CommandRecorder& cmdRec);

private:
	Camera* _camera;
	const RenderRegistry* _registry;

	PathTracePass _pathTracePass;
	NormalizationPass _normalizationPass;
	ExposurePass _exposurePass;
	BloomPass _bloomPass;
	ToneMappingPass _toneMappingPass;

	PathTracePassOutput _pathTracePassOutput{};

	bool _firstTrace{true};
};
}
