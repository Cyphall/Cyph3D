#include "LightingPass.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

#include <glm/gtc/matrix_inverse.hpp>

LightingPass::LightingPass(glm::uvec2 size):
	RenderPass(size, "Lighting pass")
{
	createUniformBuffers();
	createSamplers();
	createDescriptorSetLayouts();
	createPipelineLayout();
	createPipeline();
	createImage();
}

LightingPassOutput LightingPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, LightingPassInput& input)
{
	descriptorSetsResizeSmart(input.directionalShadowMapInfos.size(), input.pointShadowMapInfos.size());

	_directionalLightsUniforms->resizeSmart(input.registry.getDirectionalLightRenderRequests().size());
	uint32_t directionalLightShadowIndex = 0;
	for (int i = 0; i < input.registry.getDirectionalLightRenderRequests().size(); i++)
	{
		const DirectionalLight::RenderData& light = input.registry.getDirectionalLightRenderRequests()[i];

		DirectionalLightUniforms* directionalLightUniformsPtr = _directionalLightsUniforms->getHostPointer() + i;
		directionalLightUniformsPtr->fragToLightDirection = light.transform.getUp();
		directionalLightUniformsPtr->intensity = light.intensity;
		directionalLightUniformsPtr->color = light.color;
		directionalLightUniformsPtr->castShadows = light.castShadows;
		if (light.castShadows)
		{
			const DirectionalShadowMapInfo& shadowMapInfo = input.directionalShadowMapInfos[directionalLightShadowIndex];

			directionalLightUniformsPtr->lightViewProjection = shadowMapInfo.viewProjection;
			directionalLightUniformsPtr->textureIndex = directionalLightShadowIndex;
			directionalLightUniformsPtr->shadowMapTexelWorldSize = shadowMapInfo.worldSize / light.shadowMapResolution;

			_directionalLightDescriptorSet->bindDescriptor(1, shadowMapInfo.image, _directionalLightSampler, directionalLightShadowIndex);

			directionalLightShadowIndex++;
		}
	}
	_directionalLightDescriptorSet->bindDescriptor(0, _directionalLightsUniforms.getCurrent()->getBuffer(), 0, input.registry.getDirectionalLightRenderRequests().size());

	_pointLightsUniforms->resizeSmart(input.registry.getPointLightRenderRequests().size());
	uint32_t pointLightShadowIndex = 0;
	for (int i = 0; i < input.registry.getPointLightRenderRequests().size(); i++)
	{
		const PointLight::RenderData& light = input.registry.getPointLightRenderRequests()[i];

		PointLightUniforms* pointLightUniformsPtr = _pointLightsUniforms->getHostPointer() + i;
		pointLightUniformsPtr->pos = light.transform.getWorldPosition();
		pointLightUniformsPtr->intensity = light.intensity;
		pointLightUniformsPtr->color = light.color;
		pointLightUniformsPtr->castShadows = light.castShadows;
		if (light.castShadows)
		{
			const PointShadowMapInfo& shadowMapInfo = input.pointShadowMapInfos[pointLightShadowIndex];

			pointLightUniformsPtr->textureIndex = pointLightShadowIndex;
			pointLightUniformsPtr->maxTexelSizeAtUnitDistance = 2.0f / light.shadowMapResolution;

			_pointLightDescriptorSet->bindDescriptor(1, shadowMapInfo.image, _pointLightSampler, pointLightShadowIndex);

			pointLightShadowIndex++;
		}
	}
	_pointLightDescriptorSet->bindDescriptor(0, _pointLightsUniforms.getCurrent()->getBuffer(), 0, input.registry.getPointLightRenderRequests().size());

	commandBuffer->imageMemoryBarrier(
		_multisampledRawRenderImage,
		vk::PipelineStageFlagBits2::eAllCommands,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	VKRenderingInfo renderingInfo(_size);

	renderingInfo.addColorAttachment(_multisampledRawRenderImage)
		.setLoadOpClear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
		.setStoreOpStore();

	renderingInfo.setDepthAttachment(input.multisampledDepthImage)
		.setLoadOpLoad()
		.setStoreOpNone();

	commandBuffer->beginRendering(renderingInfo);

	commandBuffer->bindPipeline(_pipeline);

	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = _size;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);

	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = _size;
	commandBuffer->setScissor(scissor);

	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->bindDescriptorSet(1, _directionalLightDescriptorSet.getCurrent());
	commandBuffer->bindDescriptorSet(2, _pointLightDescriptorSet.getCurrent());

	PushConstantData pushConstantData{};
	pushConstantData.viewPos = input.camera.getPosition();
	pushConstantData.frameIndex = _frameIndex;
	commandBuffer->pushConstants(pushConstantData);

	glm::mat4 viewProjection = input.camera.getProjection() * input.camera.getView();

	_objectUniforms->resizeSmart(input.registry.getModelRenderRequests().size());
	for (int i = 0; i < input.registry.getModelRenderRequests().size(); i++)
	{
		ModelRenderer::RenderData model = input.registry.getModelRenderRequests()[i];

		const VKPtr<VKBuffer<FullVertexData>>& vertexBuffer = model.mesh.getFullVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();

		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);

		ObjectUniforms* objectUniformsPtr = _objectUniforms->getHostPointer() + i;
		objectUniformsPtr->normalMatrix = glm::inverseTranspose(glm::mat3(model.transform.getLocalToWorldMatrix()));
		objectUniformsPtr->model = model.transform.getLocalToWorldMatrix();
		objectUniformsPtr->mvp = viewProjection * model.transform.getLocalToWorldMatrix();
		objectUniformsPtr->albedoIndex = model.material.getAlbedoTextureBindlessIndex();
		objectUniformsPtr->normalIndex = model.material.getNormalTextureBindlessIndex();
		objectUniformsPtr->roughnessIndex = model.material.getRoughnessTextureBindlessIndex();
		objectUniformsPtr->metalnessIndex = model.material.getMetalnessTextureBindlessIndex();
		objectUniformsPtr->displacementIndex = model.material.getDisplacementTextureBindlessIndex();
		objectUniformsPtr->emissiveIndex = model.material.getEmissiveTextureBindlessIndex();
		objectUniformsPtr->albedoValue = MathHelper::srgbToLinear(model.material.getAlbedoValue());
		objectUniformsPtr->roughnessValue = model.material.getRoughnessValue();
		objectUniformsPtr->metalnessValue = model.material.getMetalnessValue();
		objectUniformsPtr->displacementScale = model.material.getDisplacementScale();
		objectUniformsPtr->emissiveScale = model.material.getEmissiveScale();

		commandBuffer->pushDescriptor(3, 0, _objectUniforms.getCurrent()->getBuffer(), i, 1);

		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}

	commandBuffer->unbindPipeline();

	commandBuffer->endRendering();

	_frameIndex++;

	return {
		.multisampledRawRenderImage = _multisampledRawRenderImage
	};
}

void LightingPass::onResize()
{
	createImage();
}

void LightingPass::createUniformBuffers()
{
	VKResizableBufferInfo directionalLightsUniformsBufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	directionalLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	directionalLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	directionalLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	directionalLightsUniformsBufferInfo.setName("Directional lights uniform buffer");

	_directionalLightsUniforms = VKDynamic<VKResizableBuffer<DirectionalLightUniforms>>(
		Engine::getVKContext(),
		[&](VKContext& context, int index)
		{
			return VKResizableBuffer<DirectionalLightUniforms>::create(context, directionalLightsUniformsBufferInfo);
		}
	);

	VKResizableBufferInfo pointLightsUniformsBufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	pointLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	pointLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	pointLightsUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	pointLightsUniformsBufferInfo.setName("Point lights uniform buffer");

	_pointLightsUniforms = VKDynamic<VKResizableBuffer<PointLightUniforms>>(
		Engine::getVKContext(),
		[&](VKContext& context, int index)
		{
			return VKResizableBuffer<PointLightUniforms>::create(context, pointLightsUniformsBufferInfo);
		}
	);

	VKResizableBufferInfo objectUniformsBufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	objectUniformsBufferInfo.setName("Objects uniform buffer");

	_objectUniforms = VKDynamic<VKResizableBuffer<ObjectUniforms>>(
		Engine::getVKContext(),
		[&](VKContext& context, int index)
		{
			return VKResizableBuffer<ObjectUniforms>::create(context, objectUniformsBufferInfo);
		}
	);
}

void LightingPass::createSamplers()
{
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eNearest;
		createInfo.minFilter = vk::Filter::eNearest;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToBorder;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToBorder;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToBorder;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		createInfo.unnormalizedCoordinates = false;

		_directionalLightSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}

	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eNearest;
		createInfo.minFilter = vk::Filter::eNearest;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		createInfo.unnormalizedCoordinates = false;

		_pointLightSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
}

void LightingPass::createDescriptorSetLayouts()
{
	{
		VKDescriptorSetLayoutInfo info(false);
		info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
		info.addIndexedBinding(vk::DescriptorType::eCombinedImageSampler, 1024);

		_directionalLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}

	{
		VKDescriptorSetLayoutInfo info(false);
		info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
		info.addIndexedBinding(vk::DescriptorType::eCombinedImageSampler, 1024);

		_pointLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}

	{
		VKDescriptorSetLayoutInfo info(true);
		info.addBinding(vk::DescriptorType::eStorageBuffer, 1);

		_objectDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
}

void LightingPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_directionalLightDescriptorSetLayout);
	info.addDescriptorSetLayout(_pointLightDescriptorSetLayout);
	info.addDescriptorSetLayout(_objectDescriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();

	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void LightingPass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		FileHelper::getDataDirectory() / "shaders/lighting/lighting.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise
	);

	info.setFragmentShader(FileHelper::getDataDirectory() / "shaders/lighting/lighting.frag");

	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(FullVertexData), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(FullVertexData, position));
	info.getVertexInputLayoutInfo().defineAttribute(0, 1, vk::Format::eR32G32Sfloat, offsetof(FullVertexData, uv));
	info.getVertexInputLayoutInfo().defineAttribute(0, 2, vk::Format::eR32G32B32Sfloat, offsetof(FullVertexData, normal));
	info.getVertexInputLayoutInfo().defineAttribute(0, 3, vk::Format::eR32G32B32A32Sfloat, offsetof(FullVertexData, tangent));

	info.setRasterizationSampleCount(vk::SampleCountFlagBits::e4);

	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
	info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eEqual, false);

	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void LightingPass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::HDR_COLOR_FORMAT,
		_size,
		1,
		1,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setSampleCount(vk::SampleCountFlagBits::e4);
	imageInfo.setName("Multisampled raw render image");

	_multisampledRawRenderImage = VKImage::create(Engine::getVKContext(), imageInfo);
}

void LightingPass::descriptorSetsResizeSmart(uint32_t directionalLightShadowsCount, uint32_t pointLightShadowsCount)
{
	if (!_directionalLightDescriptorSet || _directionalLightDescriptorSet->getInfo().getVariableSizeAllocatedCount() < directionalLightShadowsCount)
	{
		// resize to next power of 2
		size_t tentativeCount = 1;
		while (tentativeCount < directionalLightShadowsCount)
		{
			tentativeCount *= 2;
		}

		VKDescriptorSetInfo info(_directionalLightDescriptorSetLayout);
		info.setVariableSizeAllocatedCount(tentativeCount);

		_directionalLightDescriptorSet = VKDynamic<VKDescriptorSet>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKDescriptorSet::create(context, info);
			}
		);
	}

	if (!_pointLightDescriptorSet || _pointLightDescriptorSet->getInfo().getVariableSizeAllocatedCount() < pointLightShadowsCount)
	{
		// resize to next power of 2
		size_t tentativeCount = 1;
		while (tentativeCount < pointLightShadowsCount)
		{
			tentativeCount *= 2;
		}

		VKDescriptorSetInfo info(_pointLightDescriptorSetLayout);
		info.setVariableSizeAllocatedCount(tentativeCount);

		_pointLightDescriptorSet = VKDynamic<VKDescriptorSet>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKDescriptorSet::create(context, info);
			}
		);
	}
}