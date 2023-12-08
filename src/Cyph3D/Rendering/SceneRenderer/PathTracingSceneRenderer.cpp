#include "PathTracingSceneRenderer.h"

#include "Cyph3D/VKObject/Image/VKImage.h"

PathTracingSceneRenderer::PathTracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Path tracing SceneRenderer", size),
	_pathTracePass(size),
	_normalizationPass(size),
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
	PathTracePassInput pathTracePassInput{
		.registry = registry,
		.camera = camera,
		.sampleCount = _sampleCount,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	PathTracePassOutput pathTracePassOutput = _pathTracePass.render(commandBuffer, pathTracePassInput);

	for (int i = 0; i < 3; i++)
	{
		commandBuffer->imageMemoryBarrier(
			pathTracePassOutput.rawRenderImage[i],
			vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral
		);
	}

	NormalizationPassInput normalizationPassInput{
		.inputImage = pathTracePassOutput.rawRenderImage,
		.accumulatedSamples = pathTracePassOutput.accumulatedSamples,
		.fixedPointDecimals = pathTracePassOutput.fixedPointDecimals
	};

	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(commandBuffer, normalizationPassInput);

	commandBuffer->imageMemoryBarrier(
		normalizationPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	BloomPassInput bloomPassInput{
		.inputImage = normalizationPassOutput.outputImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput);

	commandBuffer->imageMemoryBarrier(
		bloomPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	ToneMappingPassInput toneMappingPassInput{
		.inputImage = bloomPassOutput.outputImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput);

	commandBuffer->imageMemoryBarrier(
		toneMappingPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	return toneMappingPassOutput.outputImage;
}

void PathTracingSceneRenderer::onResize()
{
	_pathTracePass.resize(_size);
	_normalizationPass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}