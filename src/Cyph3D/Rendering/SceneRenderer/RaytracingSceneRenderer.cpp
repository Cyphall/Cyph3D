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
	_objectIndexBuffer = VKBuffer<int32_t>::create(
		Engine::getVKContext(),
		1,
		vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
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
				_objectIndexImageView->getImage(),
				vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
				vk::AccessFlagBits2::eShaderStorageWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal,
				0,
				0);
			
			commandBuffer->copyPixelToBuffer(_objectIndexImageView->getImage(), 0, 0, clickPos, _objectIndexBuffer, 0);
		});
	
	int32_t* ptr = _objectIndexBuffer->map();
	int32_t objectIndex = *ptr;
	_objectIndexBuffer->unmap();
	
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
		raytracePassOutput.rawRenderImageView->getImage(),
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal,
		0,
		0);
	
	ExposurePassInput exposurePassInput{
		.inputImageView = raytracePassOutput.rawRenderImageView,
		.camera = camera
	};
	
	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		exposurePassOutput.outputImageView->getImage(),
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal,
		0,
		0);
	
	BloomPassInput bloomPassInput{
		.inputImageView = exposurePassOutput.outputImageView
	};
	
	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		bloomPassOutput.outputImageView->getImage(),
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal,
		0,
		0);
	
	ToneMappingPassInput toneMappingPassInput{
		.inputImageView = bloomPassOutput.outputImageView
	};
	
	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		toneMappingPassOutput.outputImageView->getImage(),
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal,
		0,
		0);
	
	return toneMappingPassOutput.outputImageView;
}

void RaytracingSceneRenderer::onResize()
{
	_raytracePass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}