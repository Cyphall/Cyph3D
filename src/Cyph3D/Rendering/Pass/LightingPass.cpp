#include "LightingPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
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
	createImages();
}

LightingPassOutput LightingPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, LightingPassInput& input)
{
	uint32_t directionalLightShadowsCount = 0;
	for (DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		if (renderData.castShadows)
		{
			directionalLightShadowsCount++;
		}
	}
	
	uint32_t pointLightShadowsCount = 0;
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		if (renderData.castShadows)
		{
			pointLightShadowsCount++;
		}
	}
	
	descriptorSetsResizeSmart(directionalLightShadowsCount, pointLightShadowsCount);
	
	_directionalLightsUniforms->resizeSmart(input.registry.directionalLights.size());
	uint32_t directionalLightShadowIndex = 0;
	DirectionalLightUniforms* directionalLightUniformsPtr = _directionalLightsUniforms->getHostPointer();
	for (DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		DirectionalLightUniforms uniforms{};
		uniforms.fragToLightDirection = renderData.fragToLightDirection;
		uniforms.intensity = renderData.intensity;
		uniforms.color = renderData.color;
		uniforms.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			uniforms.lightViewProjection = renderData.lightViewProjection;
			uniforms.textureIndex = directionalLightShadowIndex;
			uniforms.shadowMapTexelWorldSize = renderData.shadowMapTexelWorldSize;
			
			_directionalLightDescriptorSet->bindCombinedImageSampler(1, renderData.shadowMapTextureView->getCurrent(), _directionalLightSampler, directionalLightShadowIndex);
			
			directionalLightShadowIndex++;
		}
		
		std::memcpy(directionalLightUniformsPtr, &uniforms, sizeof(DirectionalLightUniforms));
		directionalLightUniformsPtr++;
	}
	_directionalLightDescriptorSet->bindBuffer(0, _directionalLightsUniforms.getCurrent()->getBuffer(), 0, input.registry.directionalLights.size());
	
	_pointLightsUniforms->resizeSmart(input.registry.pointLights.size());
	uint32_t pointLightShadowIndex = 0;
	PointLightUniforms* pointLightUniformsPtr = _pointLightsUniforms->getHostPointer();
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		PointLightUniforms uniforms{};
		uniforms.pos = renderData.pos;
		uniforms.intensity = renderData.intensity;
		uniforms.color = renderData.color;
		uniforms.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			uniforms.textureIndex = pointLightShadowIndex;
			uniforms.far = renderData.far;
			uniforms.maxTexelSizeAtUnitDistance = 2.0f / renderData.shadowMapResolution;
			
			_pointLightDescriptorSet->bindCombinedImageSampler(1, renderData.shadowMapTextureView->getCurrent(), _pointLightSampler, pointLightShadowIndex);
			
			pointLightShadowIndex++;
		}
		
		std::memcpy(pointLightUniformsPtr, &uniforms, sizeof(PointLightUniforms));
		pointLightUniformsPtr++;
	}
	_pointLightDescriptorSet->bindBuffer(0, _pointLightsUniforms.getCurrent()->getBuffer(), 0, input.registry.pointLights.size());
	
	commandBuffer->imageMemoryBarrier(
		_multisampledRawRenderImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	commandBuffer->imageMemoryBarrier(
		_multisampledObjectIndexImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	commandBuffer->imageMemoryBarrier(
		_resolvedObjectIndexImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	VKRenderingInfo renderingInfo(_size);
	
	renderingInfo.addColorAttachment(_multisampledRawRenderImageView.getCurrent())
		.setLoadOpClear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
		.setStoreOpStore();
	
	renderingInfo.addColorAttachment(_multisampledObjectIndexImageView.getCurrent())
		.enableResolve(vk::ResolveModeFlagBits::eSampleZero, _resolvedObjectIndexImageView.getCurrent())
		.setLoadOpClear(glm::ivec4(-1, 0, 0, 0))
		.setStoreOpStore();
	
	renderingInfo.setDepthAttachment(input.multisampledDepthImageView)
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
	pushConstantData.viewProjectionInv = glm::inverse(input.camera.getProjection() * input.camera.getView());
	pushConstantData.viewPos = input.camera.getPosition();
	pushConstantData.frameIndex = _frameIndex;
	commandBuffer->pushConstants(pushConstantData);
	
	glm::mat4 vp = input.camera.getProjection() * input.camera.getView();
	
	_objectUniforms->resizeSmart(input.registry.models.size());
	ObjectUniforms* objectUniformsPtr = _objectUniforms->getHostPointer();
	for (int i = 0; i < input.registry.models.size(); i++)
	{
		ModelRenderer::RenderData modelData = input.registry.models[i];
		
		MaterialAsset* material = modelData.material;
		if (material == nullptr)
		{
			material = MaterialAsset::getMissingMaterial();
		}
		else if (!material->isLoaded())
		{
			material = MaterialAsset::getDefaultMaterial();
		}
		
		if (material == nullptr || !material->isLoaded())
		{
			continue;
		}
		
		MeshAsset* mesh = modelData.mesh;
		if (mesh == nullptr || !mesh->isLoaded())
		{
			continue;
		}
		
		const VKPtr<VKBuffer<VertexData>>& vertexBuffer = mesh->getVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = mesh->getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		ObjectUniforms uniforms{};
		uniforms.normalMatrix = glm::inverseTranspose(glm::mat3(modelData.matrix));
		uniforms.model = modelData.matrix;
		uniforms.mvp = vp * modelData.matrix;
		uniforms.objectIndex = i;
		uniforms.albedoIndex = material->getAlbedoTextureBindlessIndex();
		uniforms.normalIndex = material->getNormalTextureBindlessIndex();
		uniforms.roughnessIndex = material->getRoughnessTextureBindlessIndex();
		uniforms.metalnessIndex = material->getMetalnessTextureBindlessIndex();
		uniforms.displacementIndex = material->getDisplacementTextureBindlessIndex();
		uniforms.emissiveIndex = material->getEmissiveTextureBindlessIndex();
		uniforms.emissiveScale = material->getEmissiveScale();
		std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
		
		commandBuffer->pushDescriptor(3, 0, _objectUniforms.getCurrent()->getBuffer(), i, 1);
		
		commandBuffer->draw(indexBuffer->getSize(), 0);
		
		objectUniformsPtr++;
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	_frameIndex++;
	
	return {
		.multisampledRawRenderImageView = _multisampledRawRenderImageView.getCurrent(),
		.objectIndexImageView = _resolvedObjectIndexImageView.getCurrent()
	};
}

void LightingPass::onResize()
{
	createImages();
}

void LightingPass::createUniformBuffers()
{
	_directionalLightsUniforms = VKDynamic<VKResizableBuffer<DirectionalLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<DirectionalLightUniforms>::create(
			context,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_pointLightsUniforms = VKDynamic<VKResizableBuffer<PointLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<PointLightUniforms>::create(
			context,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_objectUniforms = VKDynamic<VKResizableBuffer<ObjectUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<ObjectUniforms>::create(
			context,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
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
		"resources/shaders/internal/lighting/lighting.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.setFragmentShader("resources/shaders/internal/lighting/lighting.frag");
	
	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	info.getVertexInputLayoutInfo().defineAttribute(0, 1, vk::Format::eR32G32Sfloat, offsetof(VertexData, uv));
	info.getVertexInputLayoutInfo().defineAttribute(0, 2, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, normal));
	info.getVertexInputLayoutInfo().defineAttribute(0, 3, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, tangent));
	
	info.setRasterizationSampleCount(vk::SampleCountFlagBits::e4);
	
	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::OBJECT_INDEX_FORMAT);
	info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eEqual, false);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void LightingPass::createImages()
{
	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setSampleCount(vk::SampleCountFlagBits::e4);
		
		_multisampledRawRenderImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_multisampledRawRenderImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_multisampledRawRenderImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
	
	{
		VKImageInfo imageInfo(
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setSampleCount(vk::SampleCountFlagBits::e4);
		
		_multisampledObjectIndexImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_multisampledObjectIndexImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_multisampledObjectIndexImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
	
	{
		VKImageInfo imageInfo(
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_resolvedObjectIndexImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_resolvedObjectIndexImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_resolvedObjectIndexImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
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
		
		_directionalLightDescriptorSet = VKDynamic<VKDescriptorSet>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKDescriptorSet::create(context, info);
		});
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
		
		_pointLightDescriptorSet = VKDynamic<VKDescriptorSet>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKDescriptorSet::create(context, info);
		});
	}
}