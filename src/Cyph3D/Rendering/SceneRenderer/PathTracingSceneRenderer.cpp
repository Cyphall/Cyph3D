#include "PathTracingSceneRenderer.h"

#include "Cyph3D/Engine.h"
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
	PathTracePassInput pathTracePassInput{
		.registry = registry,
		.camera = camera,
		.sampleCount = _sampleCount,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	PathTracePassOutput pathTracePassOutput = _pathTracePass.render(commandBuffer, pathTracePassInput, _renderPerf);

	for (int i = 0; i < 3; i++)
	{
		commandBuffer->imageMemoryBarrier(
			pathTracePassOutput.rawRenderImage[i],
			vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral);
	}

	NormalizationPassInput normalizationPassInput{
		.inputImage = pathTracePassOutput.rawRenderImage,
		.accumulatedSamples = pathTracePassOutput.accumulatedSamples,
		.fixedPointDecimals = pathTracePassOutput.fixedPointDecimals
	};

	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(commandBuffer, normalizationPassInput, _renderPerf);

	commandBuffer->imageMemoryBarrier(
		normalizationPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);

	ExposurePassInput exposurePassInput{
		.inputImage = normalizationPassOutput.outputImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput, _renderPerf);

	commandBuffer->imageMemoryBarrier(
		exposurePassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);

	BloomPassInput bloomPassInput{
		.inputImage = exposurePassOutput.outputImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput, _renderPerf);

	commandBuffer->imageMemoryBarrier(
		bloomPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);

	ToneMappingPassInput toneMappingPassInput{
		.inputImage = bloomPassOutput.outputImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput, _renderPerf);

	commandBuffer->imageMemoryBarrier(
		toneMappingPassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);

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