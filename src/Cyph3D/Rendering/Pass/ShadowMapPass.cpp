#include "ShadowMapPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Scene/Camera.h"

ShadowMapPass::ShadowMapPass(glm::uvec2 size):
	RenderPass(size, "Shadow map pass")
{
	createDescriptorSetLayout();
	createBuffer();
	createPipelineLayouts();
	createPipelines();
}

ShadowMapPassOutput ShadowMapPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		int shadowCastingDirectionalLightIndex = 0;
		for (const DirectionalLight::RenderData& light : input.registry.getDirectionalLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Directional light ({})", shadowCastingDirectionalLightIndex));
			renderDirectionalShadowMap(commandBuffer, light, input.registry.getModelRenderRequests());
			commandBuffer->popDebugGroup();
			
			shadowCastingDirectionalLightIndex++;
		}
	}
	
	if (input.sceneChanged)
	{
		int shadowCastingPointLights = 0;
		for (const PointLight::RenderData& renderData : input.registry.getPointLightRenderRequests())
		{
			if (renderData.castShadows)
			{
				shadowCastingPointLights++;
			}
		}
		
		_pointLightUniformBuffer->resizeSmart(shadowCastingPointLights);
		
		int shadowCastingPointLightIndex = 0;
		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Point light ({})", shadowCastingPointLightIndex));
			renderPointShadowMap(commandBuffer, light, input.registry.getModelRenderRequests(), shadowCastingPointLightIndex);
			commandBuffer->popDebugGroup();
			
			shadowCastingPointLightIndex++;
		}
	}
	
	return {};
}

void ShadowMapPass::onResize()
{

}

void ShadowMapPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_pointLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ShadowMapPass::createBuffer()
{
	VKResizableBufferInfo bufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_pointLightUniformBuffer = VKDynamic<VKResizableBuffer<PointLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<PointLightUniforms>::create(context, bufferInfo);
	});
}

void ShadowMapPass::createPipelineLayouts()
{
	{
		VKPipelineLayoutInfo info;
		info.setPushConstantLayout<DirectionalLightPushConstantData>();
		
		_directionalLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_pointLightDescriptorSetLayout);
		info.setPushConstantLayout<PointLightPushConstantData>();
		
		_pointLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::createPipelines()
{
	{
		VKGraphicsPipelineInfo info(
			_directionalLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/directional light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(DirectionalLight::depthFormat, vk::CompareOp::eLess, true);
		
		_directionalLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info(
			_pointLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/point light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/shadow mapping/point light.frag");
		
		info.setViewMask(0b111111);
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(PointLight::depthFormat, vk::CompareOp::eLess, true);
		
		_pointLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::renderDirectionalShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const DirectionalLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models)
{
	commandBuffer->imageMemoryBarrier(
		(*light.shadowMapTextureView)->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	glm::uvec2 shadowMapSize = (*light.shadowMapTextureView)->getInfo().getImage()->getSize(0);
	
	VKRenderingInfo renderingInfo(shadowMapSize);
	
	renderingInfo.setDepthAttachment(*light.shadowMapTextureView)
		.setLoadOpClear(1.0f)
		.setStoreOpStore();
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_directionalLightPipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = shadowMapSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = shadowMapSize;
	commandBuffer->setScissor(scissor);
	
	for (const ModelRenderer::RenderData& model : models)
	{
		if (!model.contributeShadows)
		{
			continue;
		}
		
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh->getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh->getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		DirectionalLightPushConstantData pushConstantData{};
		pushConstantData.mvp = light.lightViewProjection * model.matrix;
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void ShadowMapPass::renderPointShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const PointLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models,
	int uniformIndex)
{
	for (int i = 0; i < 6; i++)
	{
		commandBuffer->imageMemoryBarrier(
			(*light.shadowMapTextureView)->getInfo().getImage(),
			i,
			0,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentRead,
			vk::ImageLayout::eDepthAttachmentOptimal);
	}
	
	glm::uvec2 shadowMapSize = (*light.shadowMapTextureView)->getInfo().getImage()->getSize(0);
	
	VKRenderingInfo renderingInfo(shadowMapSize);
	renderingInfo.setLayers(6);
	renderingInfo.setViewMask(0b111111);
	
	renderingInfo.setDepthAttachment(*light.shadowMapTextureView)
		.setLoadOpClear(std::numeric_limits<float>::max())
		.setStoreOpStore();
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_pointLightPipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = shadowMapSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = shadowMapSize;
	commandBuffer->setScissor(scissor);
	
	PointLightUniforms uniforms{
		.viewProjections = {
			light.viewProjections[0],
			light.viewProjections[1],
			light.viewProjections[2],
			light.viewProjections[3],
			light.viewProjections[4],
			light.viewProjections[5]
		},
		.lightPos = light.pos
	};
	std::memcpy(_pointLightUniformBuffer->getHostPointer() + uniformIndex, &uniforms, sizeof(PointLightUniforms));
	
	commandBuffer->pushDescriptor(0, 0, _pointLightUniformBuffer.getCurrent()->getBuffer(), uniformIndex, 1);
	
	for (const ModelRenderer::RenderData& model : models)
	{
		if (!model.contributeShadows)
		{
			continue;
		}
		
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh->getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh->getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		PointLightPushConstantData pushConstantData{};
		pushConstantData.model = model.matrix;
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}
