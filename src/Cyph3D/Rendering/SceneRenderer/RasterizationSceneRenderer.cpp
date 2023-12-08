#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_skyboxPass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
}

const VKPtr<VKImage>& RasterizationSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	ZPrepassInput zPrepassInput{
		.registry = registry,
		.camera = camera
	};

	ZPrepassOutput zPrepassOutput = _zPrepass.render(commandBuffer, zPrepassInput);

	ShadowMapPassInput shadowMapPassInput{
		.registry = registry,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	ShadowMapPassOutput shadowMapPassOutput = _shadowMapPass.render(commandBuffer, shadowMapPassInput);

	commandBuffer->imageMemoryBarrier(
		zPrepassOutput.multisampledDepthImage,
		vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentRead,
		vk::ImageLayout::eDepthAttachmentOptimal
	);

	for (const DirectionalShadowMapInfo& info : shadowMapPassOutput.directionalShadowMapInfos)
	{
		commandBuffer->imageMemoryBarrier(
			info.image,
			vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal
		);
	}

	for (const PointShadowMapInfo& info : shadowMapPassOutput.pointShadowMapInfos)
	{
		commandBuffer->imageMemoryBarrier(
			info.image,
			vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal
		);
	}

	LightingPassInput lightingPassInput{
		.multisampledDepthImage = zPrepassOutput.multisampledDepthImage,
		.registry = registry,
		.camera = camera,
		.directionalShadowMapInfos = shadowMapPassOutput.directionalShadowMapInfos,
		.pointShadowMapInfos = shadowMapPassOutput.pointShadowMapInfos
	};

	LightingPassOutput lightingPassOutput = _lightingPass.render(commandBuffer, lightingPassInput);

	commandBuffer->imageMemoryBarrier(
		lightingPassOutput.multisampledRawRenderImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentRead,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	SkyboxPassInput skyboxPassInput{
		.camera = camera,
		.multisampledRawRenderImage = lightingPassOutput.multisampledRawRenderImage,
		.multisampledDepthImage = zPrepassOutput.multisampledDepthImage
	};

	SkyboxPassOutput skyboxPassOutput = _skyboxPass.render(commandBuffer, skyboxPassInput);

	commandBuffer->imageMemoryBarrier(
		skyboxPassOutput.rawRenderImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	BloomPassInput bloomPassInput{
		.inputImage = skyboxPassOutput.rawRenderImage
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

void RasterizationSceneRenderer::onResize()
{
	_zPrepass.resize(_size);
	_shadowMapPass.resize(_size);
	_lightingPass.resize(_size);
	_skyboxPass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}