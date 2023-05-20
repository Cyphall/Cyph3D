#include "RaytracingSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

RaytracingSceneRenderer::RaytracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Raytracing SceneRenderer", size),
	_raytracePass(size),
	_postProcessingPass(size)
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
	
	PostProcessingPassInput postProcessingPassInput{
		.rawRenderImageView = raytracePassOutput.rawRenderImageView,
		.camera = camera
	};
	
	PostProcessingPassOutput postProcessingPassOutput = _postProcessingPass.render(commandBuffer, postProcessingPassInput, _renderPerf);
	
	return postProcessingPassOutput.postProcessedRenderImageView;
}

void RaytracingSceneRenderer::onResize()
{
	_raytracePass.resize(_size);
	_postProcessingPass.resize(_size);
}