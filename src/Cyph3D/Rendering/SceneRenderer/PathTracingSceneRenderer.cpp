#include "PathTracingSceneRenderer.h"

#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

PathTracingSceneRenderer::PathTracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Path tracing SceneRenderer", size),
	_pathTracePass(size),
	_normalizationPass(size),
	_exposurePass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
}

void PathTracingSceneRenderer::setSampleCountPerRender(uint32_t count)
{
	_sampleCount = count;
}

const VKPtr<VKImage>& PathTracingSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	// Path trace pass

	PathTracePassInput pathTracePassInput{
		.registry = registry,
		.camera = camera,
		.sampleCount = _sampleCount,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	PathTracePassOutput pathTracePassOutput = _pathTracePass.render(commandBuffer, pathTracePassInput);

	// Normalization pass

	NormalizationPassInput normalizationPassInput{
		.inputImage = pathTracePassOutput.rawRenderImage,
		.accumulatedSamples = pathTracePassOutput.accumulatedSamples,
		.fixedPointDecimals = pathTracePassOutput.fixedPointDecimals
	};

	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(commandBuffer, normalizationPassInput);

	// Exposure pass

	ExposurePassInput exposurePassInput{
		.inputImage = normalizationPassOutput.outputImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput);

	// Bloom pass

	BloomPassInput bloomPassInput{
		.inputImage = exposurePassOutput.outputImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput);

	// Tone mapping pass

	ToneMappingPassInput toneMappingPassInput{
		.inputImage = bloomPassOutput.outputImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput);

	return toneMappingPassOutput.outputImage;
}

void PathTracingSceneRenderer::onResize()
{
	_pathTracePass.resize(_size);
	_normalizationPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}