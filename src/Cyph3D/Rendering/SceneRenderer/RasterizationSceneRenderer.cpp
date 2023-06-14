#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_skyboxPass(size),
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
	
	commandBuffer->imageMemoryBarrier(
		zPrepassOutput.multisampledDepthImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentRead,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	for (DirectionalLight::RenderData& renderData : _registry.directionalLights)
	{
		if (renderData.castShadows)
		{
			commandBuffer->imageMemoryBarrier(
				(*renderData.shadowMapTextureView)->getInfo().getImage(),
				0,
				0,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal);
		}
	}
	
	for (PointLight::RenderData& renderData : _registry.pointLights)
	{
		if (renderData.castShadows)
		{
			for (int i = 0; i < 6; i++)
			{
				commandBuffer->imageMemoryBarrier(
					renderData.shadowMapTexture->getCurrent(),
					i,
					0,
					vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
					vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
					vk::PipelineStageFlagBits2::eFragmentShader,
					vk::AccessFlagBits2::eShaderSampledRead,
					vk::ImageLayout::eReadOnlyOptimal);
			}
		}
	}
	
	LightingPassInput lightingPassInput{
		.multisampledDepthImageView = zPrepassOutput.multisampledDepthImageView,
		.registry = _registry,
		.camera = camera
	};
	
	LightingPassOutput lightingPassOutput = _lightingPass.render(commandBuffer, lightingPassInput, _renderPerf);
	_objectIndexImageView = lightingPassOutput.objectIndexImageView;
	
	commandBuffer->imageMemoryBarrier(
		lightingPassOutput.multisampledRawRenderImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentRead,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	SkyboxPassInput skyboxPassInput{
		.camera = camera,
		.multisampledRawRenderImageView = lightingPassOutput.multisampledRawRenderImageView,
		.multisampledDepthImageView = zPrepassOutput.multisampledDepthImageView
	};
	
	SkyboxPassOutput skyboxPassOutput = _skyboxPass.render(commandBuffer, skyboxPassInput, _renderPerf);
	
	commandBuffer->imageMemoryBarrier(
		skyboxPassOutput.rawRenderImageView->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	ExposurePassInput exposurePassInput{
		.inputImageView = skyboxPassOutput.rawRenderImageView,
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

Entity* RasterizationSceneRenderer::getClickedEntity(glm::uvec2 clickPos)
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
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal);
			
			commandBuffer->copyPixelToBuffer(_objectIndexImageView->getInfo().getImage(), 0, 0, clickPos, _objectIndexBuffer, 0);
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

void RasterizationSceneRenderer::onResize()
{
	_zPrepass.resize(_size);
	_shadowMapPass.resize(_size);
	_lightingPass.resize(_size);
	_skyboxPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}