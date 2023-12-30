#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/Rendering/VertexData.h"
#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

#include <glm/gtc/matrix_inverse.hpp>

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
}

void RasterizationSceneRenderer::fillSceneBuffers(const RenderRegistry& registry)
{
	if (registry.getModelRenderRequests().empty())
	{
		_modelDataBuffer = {};
		_allDrawCommandsBuffer = {};
		_shadowDrawCommandsBuffer = {};
		return;
	}

	{
		VKBufferInfo modelDataBufferInfo(registry.getModelRenderRequests().size(), vk::BufferUsageFlagBits::eShaderDeviceAddress);
		modelDataBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		modelDataBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		modelDataBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

		_modelDataBuffer = VKBuffer<RasterizationModelData>::create(Engine::getVKContext(), modelDataBufferInfo);
	}

	{
		VKBufferInfo modelDrawCommandsBufferInfo(registry.getModelRenderRequests().size(), vk::BufferUsageFlagBits::eIndirectBuffer);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

		_allDrawCommandsBuffer = VKBuffer<vk::DrawIndirectCommand>::create(Engine::getVKContext(), modelDrawCommandsBufferInfo);
	}

	{
		VKBufferInfo modelDrawCommandsBufferInfo(registry.getModelRenderRequests().size(), vk::BufferUsageFlagBits::eIndirectBuffer);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		modelDrawCommandsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

		_shadowDrawCommandsBuffer = VKBuffer<vk::DrawIndirectCommand>::create(Engine::getVKContext(), modelDrawCommandsBufferInfo);
	}

	RasterizationModelData* modelDataBufferPtr = _modelDataBuffer->getHostPointer();
	vk::DrawIndirectCommand* allDrawCommandsBufferPtr = _allDrawCommandsBuffer->getHostPointer();
	vk::DrawIndirectCommand* shadowDrawCommandsBufferPtr = _shadowDrawCommandsBuffer->getHostPointer();
	for (const ModelRenderer::RenderData& model : registry.getModelRenderRequests())
	{
		const VKPtr<VKBuffer<PositionVertexData>>& positionVertexBuffer = model.mesh.getPositionVertexBuffer();
		const VKPtr<VKBuffer<FullVertexData>>& fullVertexBuffer = model.mesh.getFullVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();

		RasterizationModelData modelData{
			.modelMatrix = model.transform.getLocalToWorldMatrix(),
			.normalMatrix = glm::inverseTranspose(model.transform.getLocalToWorldMatrix()),
			.positionVertexBuffer = positionVertexBuffer->getDeviceAddress(),
			.fullVertexBuffer = fullVertexBuffer->getDeviceAddress(),
			.indexBuffer = indexBuffer->getDeviceAddress(),
			.albedoIndex = model.material.getAlbedoTextureBindlessIndex(),
			.normalIndex = model.material.getNormalTextureBindlessIndex(),
			.roughnessIndex = model.material.getRoughnessTextureBindlessIndex(),
			.metalnessIndex = model.material.getMetalnessTextureBindlessIndex(),
			.displacementIndex = model.material.getDisplacementTextureBindlessIndex(),
			.emissiveIndex = model.material.getEmissiveTextureBindlessIndex(),
			.albedoValue = MathHelper::srgbToLinear(model.material.getAlbedoValue()),
			.roughnessValue = model.material.getRoughnessValue(),
			.metalnessValue = model.material.getMetalnessValue(),
			.displacementScale = model.material.getDisplacementScale(),
			.emissiveScale = model.material.getEmissiveScale()
		};
		std::memcpy(modelDataBufferPtr++, &modelData, sizeof(RasterizationModelData));

		vk::DrawIndirectCommand drawCommand(
			indexBuffer->getSize(),
			1,
			0,
			0
		);
		std::memcpy(allDrawCommandsBufferPtr++, &drawCommand, sizeof(vk::DrawIndirectCommand));

		vk::DrawIndirectCommand shadowDrawCommand(
			indexBuffer->getSize(),
			model.contributeShadows ? 1 : 0,
			0,
			0
		);
		std::memcpy(shadowDrawCommandsBufferPtr++, &shadowDrawCommand, sizeof(vk::DrawIndirectCommand));
	}
}

const VKPtr<VKImage>& RasterizationSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	if (sceneChanged)
	{
		fillSceneBuffers(registry);
	}

	ZPrepassInput zPrepassInput{
		.modelDataBuffer = _modelDataBuffer,
		.drawCommandsBuffer = _allDrawCommandsBuffer,
		.camera = camera
	};

	ZPrepassOutput zPrepassOutput = _zPrepass.render(commandBuffer, zPrepassInput);

	ShadowMapPassInput shadowMapPassInput{
		.registry = registry,
		.modelDataBuffer = _modelDataBuffer,
		.drawCommandsBuffer = _shadowDrawCommandsBuffer,
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
		.modelDataBuffer = _modelDataBuffer,
		.drawCommandsBuffer = _allDrawCommandsBuffer,
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

	ExposurePassInput exposurePassInput{
		.inputImage = skyboxPassOutput.rawRenderImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput);

	commandBuffer->imageMemoryBarrier(
		exposurePassOutput.outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	BloomPassInput bloomPassInput{
		.inputImage = exposurePassOutput.outputImage
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
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}