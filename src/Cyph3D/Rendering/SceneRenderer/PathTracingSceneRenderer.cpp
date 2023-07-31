#include "PathTracingSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"

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

const VKPtr<VKImageView>& PathTracingSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	PathTracePassInput pathTracePassInput{
		.registry = registry,
		.camera = camera,
		.sampleCount = _sampleCount,
		.resetAccumulation = sceneChanged || cameraChanged
	};
	
	PathTracePassOutput pathTracePassOutput = _pathTracePass.render(commandBuffer, pathTracePassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		pathTracePassOutput.rawRenderImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	NormalizationPassInput normalizationPassInput{
		.inputImageView = pathTracePassOutput.rawRenderImageView,
		.accumulatedSamples = pathTracePassOutput.accumulatedSamples
	};
	
	NormalizationPassOutput normalizationPassOutput = _normalizationPass.render(commandBuffer, normalizationPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		normalizationPassOutput.outputImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	ExposurePassInput exposurePassInput{
		.inputImageView = normalizationPassOutput.outputImageView,
		.camera = camera
	};
	
	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		exposurePassOutput.outputImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	BloomPassInput bloomPassInput{
		.inputImageView = exposurePassOutput.outputImageView
	};
	
	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		bloomPassOutput.outputImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	ToneMappingPassInput toneMappingPassInput{
		.inputImageView = bloomPassOutput.outputImageView
	};
	
	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		toneMappingPassOutput.outputImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	return toneMappingPassOutput.outputImageView;
}

void PathTracingSceneRenderer::onResize()
{
	_pathTracePass.resize(_size);
	_normalizationPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}