#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_skyboxPass(size),
	_postProcessingPass(size)
{
	_objectIndexBuffer = VKBuffer<int32_t>::create(
		Engine::getVKContext(),
		1,
		vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
}

const VKPtr<VKImageView>& RasterizationSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera)
{
	ZPrepassInput zPrepassInput{
		.registry = _registry,
		.camera = camera
	};
	
	ZPrepassOutput zPrepassOutput = _zPrepass.render(commandBuffer, zPrepassInput, _renderPerf);
	
	ShadowMapPassInput shadowMapPassInput{
		.registry = _registry,
		.camera = camera
	};
	
	_shadowMapPass.render(commandBuffer, shadowMapPassInput, _renderPerf);
	
	LightingPassInput lightingPassInput{
		.depthView = zPrepassOutput.depthView,
		.registry = _registry,
		.camera = camera
	};
	
	LightingPassOutput lightingPassOutput = _lightingPass.render(commandBuffer, lightingPassInput, _renderPerf);
	_objectIndexView = lightingPassOutput.objectIndexView;
	
	Scene& scene = Engine::getScene();
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isLoaded())
	{
		SkyboxPassInput skyboxPassInput{
			.camera = camera,
			.rawRenderView = lightingPassOutput.rawRenderView,
			.depthView = zPrepassOutput.depthView
		};
		
		_skyboxPass.render(commandBuffer, skyboxPassInput, _renderPerf);
	}
	
	commandBuffer->imageMemoryBarrier(
		lightingPassOutput.rawRenderView->getImage(),
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal,
		0,
		0);
	
	PostProcessingPassInput postProcessingPassInput{
		.rawRenderView = lightingPassOutput.rawRenderView,
		.camera = camera
	};
	
	PostProcessingPassOutput postProcessingPassOutput = _postProcessingPass.render(commandBuffer, postProcessingPassInput, _renderPerf);
	
	return postProcessingPassOutput.postProcessedRenderView;
}

Entity* RasterizationSceneRenderer::getClickedEntity(glm::uvec2 clickPos)
{
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				_objectIndexView->getImage(),
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal,
				0,
				0);
			
			commandBuffer->copyPixelToBuffer(_objectIndexView->getImage(), 0, 0, clickPos, _objectIndexBuffer, 0);
		});
	
	int32_t* ptr = _objectIndexBuffer->map();
	int32_t objectIndex = *ptr;
	_objectIndexBuffer->unmap();
	
	if (objectIndex != -1)
	{
		return _registry.shapes[objectIndex].owner;
	}
	
	return nullptr;
}