#include "LightingPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
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
	DirectionalLightUniforms* directionalLightUniformsPtr = _directionalLightsUniforms->map();
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
			uniforms.mapSize = renderData.shadowMapSize;
			uniforms.mapDepth = renderData.shadowMapDepth;
			
			commandBuffer->imageMemoryBarrier(
				renderData.shadowMapTexture->getVKPtr(),
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal,
				0,
				0);
			
			_directionalLightDescriptorSet->bindCombinedImageSampler(1, renderData.shadowMapTextureView->getVKPtr(), _directionalLightSampler, directionalLightShadowIndex);
			
			directionalLightShadowIndex++;
		}
		
		std::memcpy(directionalLightUniformsPtr, &uniforms, sizeof(DirectionalLightUniforms));
		directionalLightUniformsPtr++;
	}
	_directionalLightsUniforms->unmap();
	_directionalLightDescriptorSet->bindBuffer(0, _directionalLightsUniforms.getVKPtr()->getBuffer(), 0, input.registry.directionalLights.size());
	
	_pointLightsUniforms->resizeSmart(input.registry.pointLights.size());
	uint32_t pointLightShadowIndex = 0;
	PointLightUniforms* pointLightUniformsPtr = _pointLightsUniforms->map();
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
			
			for (int i = 0; i < 6; i++)
			{
				commandBuffer->imageMemoryBarrier(
					renderData.shadowMapTexture->getVKPtr(),
					vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
					vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
					vk::PipelineStageFlagBits2::eFragmentShader,
					vk::AccessFlagBits2::eShaderSampledRead,
					vk::ImageLayout::eReadOnlyOptimal,
					i,
					0);
			}
			
			_pointLightDescriptorSet->bindCombinedImageSampler(1, renderData.shadowMapTextureView->getVKPtr(), _pointLightSampler, pointLightShadowIndex);
			
			pointLightShadowIndex++;
		}
		
		std::memcpy(pointLightUniformsPtr, &uniforms, sizeof(PointLightUniforms));
		pointLightUniformsPtr++;
	}
	_pointLightsUniforms->unmap();
	_pointLightDescriptorSet->bindBuffer(0, _pointLightsUniforms.getVKPtr()->getBuffer(), 0, input.registry.pointLights.size());
	
	commandBuffer->imageMemoryBarrier(
		_rawRenderImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal,
		0,
		0);
	
	commandBuffer->imageMemoryBarrier(
		_objectIndexImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal,
		0,
		0);
	
	commandBuffer->imageMemoryBarrier(
		input.depthImageView->getImage(),
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentRead,
		vk::ImageLayout::eDepthAttachmentOptimal,
		0,
		0);
	
	std::vector<vk::RenderingAttachmentInfo> colorAttachments;
	
	vk::RenderingAttachmentInfo& rawRenderAttachment = colorAttachments.emplace_back();
	rawRenderAttachment.imageView = _rawRenderImageView->getHandle();
	rawRenderAttachment.imageLayout = _rawRenderImageView->getImage()->getLayout(0, 0);
	rawRenderAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	rawRenderAttachment.resolveImageView = nullptr;
	rawRenderAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	rawRenderAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	rawRenderAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	rawRenderAttachment.clearValue.color.float32[0] = 0.0f;
	rawRenderAttachment.clearValue.color.float32[1] = 0.0f;
	rawRenderAttachment.clearValue.color.float32[2] = 0.0f;
	rawRenderAttachment.clearValue.color.float32[3] = 1.0f;
	
	vk::RenderingAttachmentInfo& objectIndexAttachment = colorAttachments.emplace_back();
	objectIndexAttachment.imageView = _objectIndexImageView->getHandle();
	objectIndexAttachment.imageLayout = _objectIndexImageView->getImage()->getLayout(0, 0);
	objectIndexAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	objectIndexAttachment.resolveImageView = nullptr;
	objectIndexAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	objectIndexAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	objectIndexAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	objectIndexAttachment.clearValue.color.int32[0] = -1;
	
	vk::RenderingAttachmentInfo depthAttachment;
	depthAttachment.imageView = input.depthImageView->getHandle();
	depthAttachment.imageLayout = input.depthImageView->getImage()->getLayout(0, 0);
	depthAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	depthAttachment.resolveImageView = nullptr;
	depthAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eNone;
	depthAttachment.clearValue.depthStencil.depth = 1.0f;
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(_size.x, _size.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = colorAttachments.size();
	renderingInfo.pColorAttachments = colorAttachments.data();
	renderingInfo.pDepthAttachment = &depthAttachment;
	renderingInfo.pStencilAttachment = nullptr;
	
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
	
	commandBuffer->bindDescriptorSet(0, _directionalLightDescriptorSet.getVKPtr());
	commandBuffer->bindDescriptorSet(1, _pointLightDescriptorSet.getVKPtr());
	
	PushConstantData pushConstantData{};
	pushConstantData.viewProjectionInv = glm::inverse(input.camera.getProjection() * input.camera.getView());
	pushConstantData.viewPos = input.camera.getPosition();
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eFragment, pushConstantData);
	
	glm::mat4 vp = input.camera.getProjection() * input.camera.getView();
	
	_objectUniforms->resizeSmart(input.registry.models.size());
	ObjectUniforms* objectUniformsPtr = _objectUniforms->map();
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
		std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
		
		commandBuffer->pushDescriptor(2, 0, _objectUniforms.getVKPtr()->getBuffer(), i, 1);
		commandBuffer->pushDescriptor(2, 1, material->getAlbedoTextureView(), _materialSampler);
		commandBuffer->pushDescriptor(2, 2, material->getNormalTextureView(), _materialSampler);
		commandBuffer->pushDescriptor(2, 3, material->getRoughnessTextureView(), _materialSampler);
		commandBuffer->pushDescriptor(2, 4, material->getMetalnessTextureView(), _materialSampler);
		commandBuffer->pushDescriptor(2, 5, material->getDisplacementTextureView(), _materialSampler);
		commandBuffer->pushDescriptor(2, 6, material->getEmissiveTextureView(), _materialSampler);
		
		commandBuffer->draw(indexBuffer->getSize(), 0);
		
		objectUniformsPtr++;
	}
	_objectUniforms->unmap();
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {
		.rawRenderImageView = _rawRenderImageView.getVKPtr(),
		.objectIndexImageView = _objectIndexImageView.getVKPtr()
	};
}

void LightingPass::onResize()
{
	createImages();
}

void LightingPass::createUniformBuffers()
{
	_directionalLightsUniforms = VKResizableBuffer<DirectionalLightUniforms>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_pointLightsUniforms = VKResizableBuffer<PointLightUniforms>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_objectUniforms = VKResizableBuffer<ObjectUniforms>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
	
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = true;
		createInfo.maxAnisotropy = 16;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;
		
		_materialSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
}

void LightingPass::createDescriptorSetLayouts()
{
	{
		VKDescriptorSetLayoutInfo info(false);
		info.registerBinding(0, vk::DescriptorType::eStorageBuffer, 1);
		info.registerIndexedBinding(1, vk::DescriptorType::eCombinedImageSampler, 1024);
		
		_directionalLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(false);
		info.registerBinding(0, vk::DescriptorType::eStorageBuffer, 1);
		info.registerIndexedBinding(1, vk::DescriptorType::eCombinedImageSampler, 1024);
		
		_pointLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(true);
		info.registerBinding(0, vk::DescriptorType::eStorageBuffer, 1);
		info.registerBinding(1, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(2, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(3, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(4, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(5, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(6, vk::DescriptorType::eCombinedImageSampler, 1);
		
		_objectDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
}

void LightingPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.registerDescriptorSetLayout(_directionalLightDescriptorSetLayout);
	info.registerDescriptorSetLayout(_pointLightDescriptorSetLayout);
	info.registerDescriptorSetLayout(_objectDescriptorSetLayout);
	info.registerPushConstantLayout<PushConstantData>(vk::ShaderStageFlagBits::eFragment);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void LightingPass::createPipeline()
{
	VKGraphicsPipelineInfo info;
	info.vertexShaderFile = "resources/shaders/internal/lighting/lighting.vert";
	info.geometryShaderFile = std::nullopt;
	info.fragmentShaderFile = "resources/shaders/internal/lighting/lighting.frag";
	
	info.vertexInputLayoutInfo.defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	info.vertexInputLayoutInfo.defineAttribute(0, 1, vk::Format::eR32G32Sfloat, offsetof(VertexData, uv));
	info.vertexInputLayoutInfo.defineAttribute(0, 2, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, normal));
	info.vertexInputLayoutInfo.defineAttribute(0, 3, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, tangent));
	info.vertexInputLayoutInfo.defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
	
	info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
	
	info.pipelineLayout = _pipelineLayout;
	
	info.viewport = std::nullopt;
	
	info.scissor = std::nullopt;
	
	info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
	info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	
	info.pipelineAttachmentInfo.registerColorAttachment(0, SceneRenderer::HDR_COLOR_FORMAT);
	info.pipelineAttachmentInfo.registerColorAttachment(1, SceneRenderer::OBJECT_INDEX_FORMAT);
	info.pipelineAttachmentInfo.setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eEqual, false);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void LightingPass::createImages()
{
	{
		_rawRenderImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_rawRenderImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_rawRenderImage,
			vk::ImageViewType::e2D);
	}
	
	{
		_objectIndexImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_objectIndexImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_objectIndexImage,
			vk::ImageViewType::e2D);
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
		
		_directionalLightDescriptorSet = VKDescriptorSet::createDynamic(Engine::getVKContext(), info);
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
		
		_pointLightDescriptorSet = VKDescriptorSet::createDynamic(Engine::getVKContext(), info);
	}
}