#include "PathTracingSceneRenderer.h"

#include <Cyph3D/Rendering/Pass/BloomPass.h>
#include <Cyph3D/Rendering/Pass/ExposurePass.h>
#include <Cyph3D/Rendering/Pass/ToneMappingPass.h>

c3d::PathTracingSceneRenderer::PathTracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Path tracing SceneRenderer", size),
	_pathTracePass(size),
	_normalizationPass(size),
	_exposurePass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
}

void c3d::PathTracingSceneRenderer::setSampleCountPerRender(uint32_t count)
{
	_sampleCount = count;
}

void c3d::PathTracingSceneRenderer::setAccumulationOnlyMode(bool enabled)
{
	_accumulationOnlyMode = enabled;
}

cgpu::ImagePtr c3d::PathTracingSceneRenderer::onRender(cgpu::CommandRecorder& commandRecorder, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	// Path trace pass

	PathTracePassInput pathTracePassInput{
		.registry = registry,
		.camera = camera,
		.sampleCount = _sampleCount,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	PathTracePassOutput pathTracePassOutput = _pathTracePass.render(commandRecorder, pathTracePassInput);

	if (_accumulationOnlyMode)
	{
		return {};
	}

	// Normalization pass

	NormalizationPassInput normalizationPassInput{
		.lightImage = pathTracePassOutput.lightImage,
		.accumulatedSamples = pathTracePassOutput.accumulatedSamples
	};

	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(commandRecorder, normalizationPassInput);

	// Exposure pass

	ExposurePassInput exposurePassInput{
		.lightImage = normalizationPassOutput.lightImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandRecorder, exposurePassInput);

	// Bloom pass

	BloomPassInput bloomPassInput{
		.lightImage = exposurePassOutput.lightImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandRecorder, bloomPassInput);

	// Tone mapping pass

	ToneMappingPassInput toneMappingPassInput{
		.lightImage = bloomPassOutput.lightImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandRecorder, toneMappingPassInput);

	return toneMappingPassOutput.colorImage;
}

void c3d::PathTracingSceneRenderer::onResize()
{
	_pathTracePass.resize(_size);
	_normalizationPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}
