#include "OfflineRenderer.h"

#include <Cyph3D/Rendering/Pass/BloomPass.h>
#include <Cyph3D/Rendering/Pass/ExposurePass.h>
#include <Cyph3D/Rendering/Pass/ToneMappingPass.h>

c3d::OfflineRenderer::OfflineRenderer(glm::uvec2 size, Camera& camera, const RenderRegistry& registry):
	_camera{&camera},
	_registry{&registry},
	_pathTracePass{size},
	_normalizationPass{size},
	_exposurePass{size},
	_bloomPass{size},
	_toneMappingPass{size}
{
}

void c3d::OfflineRenderer::traceRays(cgpu::CommandRecorder& cmdRec, uint32_t rayCount)
{
	// Path trace pass

	PathTracePassInput pathTracePassInput{
		.registry = *_registry,
		.camera = *_camera,
		.sampleCount = rayCount,
		.sceneChanged = _firstTrace,
		.cameraChanged = _firstTrace
	};

	_pathTracePassOutput = _pathTracePass.render(cmdRec, pathTracePassInput);

	_firstTrace = false;
}

cgpu::ImagePtr c3d::OfflineRenderer::postProcess(cgpu::CommandRecorder& cmdRec)
{
	// Normalization pass

	NormalizationPassInput normalizationPassInput{
		.lightImage = _pathTracePassOutput.lightImage,
		.accumulatedSamples = _pathTracePassOutput.accumulatedSamples
	};

	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(cmdRec, normalizationPassInput);

	// Exposure pass

	ExposurePassInput exposurePassInput{
		.lightImage = normalizationPassOutput.lightImage,
		.camera = *_camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(cmdRec, exposurePassInput);

	// Bloom pass

	BloomPassInput bloomPassInput{
		.lightImage = exposurePassOutput.lightImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(cmdRec, bloomPassInput);

	// Tone mapping pass

	ToneMappingPassInput toneMappingPassInput{
		.lightImage = bloomPassOutput.lightImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(cmdRec, toneMappingPassInput);

	return toneMappingPassOutput.colorImage;
}
