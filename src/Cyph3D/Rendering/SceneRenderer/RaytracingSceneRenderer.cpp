#include "RaytracingSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"

RaytracingSceneRenderer::RaytracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Raytracing SceneRenderer", size),
	_raytracePass(size),
	_exposurePass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
	VKBufferInfo bufferInfo(1, vk::BufferUsageFlagBits::eTransferDst);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);
	
	_objectIndexBuffer = VKBuffer<int32_t>::create(Engine::getVKContext(), bufferInfo);
}

Entity* RaytracingSceneRenderer::getClickedEntity(glm::uvec2 clickPos)
{
	if (!_objectIndexImageView)
	{
		return nullptr;
	}
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				_objectIndexImageView->getInfo().getImage(),
				0,
				0,
				vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
				vk::AccessFlagBits2::eShaderStorageWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal);
			
			commandBuffer->copyPixelToBuffer(_objectIndexImageView->getInfo().getImage(), 0, 0, clickPos, _objectIndexBuffer, 0);
		});
	
	int32_t objectIndex = *_objectIndexBuffer->getHostPointer();
	
	if (objectIndex != -1)
	{
		return _registry.models[objectIndex].owner;
	}
	
	return nullptr;
}

const VKPtr<VKImageView>& RaytracingSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera)
{
	RaytracePassInput raytracePassInput{
		.registry = _registry,
		.camera = camera
	};
	
	RaytracePassOutput raytracePassOutput = _raytracePass.render(commandBuffer, raytracePassInput, _renderPerf);
	_objectIndexImageView = raytracePassOutput.objectIndexImageView;
	
	commandBuffer->imageMemoryBarrier(
		raytracePassOutput.rawRenderImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	ExposurePassInput exposurePassInput{
		.inputImageView = raytracePassOutput.rawRenderImageView,
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

void RaytracingSceneRenderer::onResize()
{
	_raytracePass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}