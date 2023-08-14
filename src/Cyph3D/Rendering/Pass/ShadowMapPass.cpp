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
		for (const DirectionalLight::RenderData& directionalLightRenderData : input.registry.getDirectionalLightRenderRequests())
		{
			if (!directionalLightRenderData.castShadows)
			{
				continue;
			}
			
			commandBuffer->imageMemoryBarrier(
				(*directionalLightRenderData.shadowMapTextureView)->getInfo().getImage(),
				0,
				0,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::ImageLayout::eDepthAttachmentOptimal);
			
			glm::uvec2 shadowMapSize = (*directionalLightRenderData.shadowMapTextureView)->getInfo().getImage()->getSize(0);
			
			VKRenderingInfo renderingInfo(shadowMapSize);
			
			renderingInfo.setDepthAttachment(*directionalLightRenderData.shadowMapTextureView)
				.setLoadOpClear(1.0f)
				.setStoreOpStore();
			
			commandBuffer->pushDebugGroup(std::format("Directional light ({})", shadowCastingDirectionalLightIndex));
			
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
			
			for (const ModelRenderer::RenderData& modelRendererRenderData : input.registry.getModelRenderRequests())
			{
				if (!modelRendererRenderData.contributeShadows)
				{
					continue;
				}
				
				const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = modelRendererRenderData.mesh->getPositionVertexBuffer();
				const VKPtr<VKBuffer<uint32_t>>& indexBuffer = modelRendererRenderData.mesh->getIndexBuffer();
				
				commandBuffer->bindVertexBuffer(0, vertexBuffer);
				commandBuffer->bindIndexBuffer(indexBuffer);
				
				DirectionalLightPushConstantData pushConstantData{};
				pushConstantData.mvp = directionalLightRenderData.lightViewProjection * modelRendererRenderData.matrix;
				commandBuffer->pushConstants(pushConstantData);
				
				commandBuffer->draw(indexBuffer->getSize(), 0);
			}
			
			commandBuffer->unbindPipeline();
			
			commandBuffer->endRendering();
			
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
		
		int shadowCastingPointLightIndex = 0;
		_pointLightUniformBuffer->resizeSmart(shadowCastingPointLights);
		PointLightUniforms* pointLightUniformBufferPtr = _pointLightUniformBuffer->getHostPointer();
		for (const PointLight::RenderData& pointLightRenderData : input.registry.getPointLightRenderRequests())
		{
			if (!pointLightRenderData.castShadows)
			{
				continue;
			}
			
			for (int i = 0; i < 6; i++)
			{
				commandBuffer->imageMemoryBarrier(
					(*pointLightRenderData.shadowMapTextureView)->getInfo().getImage(),
					i,
					0,
					vk::PipelineStageFlagBits2::eNone,
					vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
					vk::AccessFlagBits2::eDepthStencilAttachmentRead,
					vk::ImageLayout::eDepthAttachmentOptimal);
			}
			
			glm::uvec2 shadowMapSize = (*pointLightRenderData.shadowMapTextureView)->getInfo().getImage()->getSize(0);
			
			VKRenderingInfo renderingInfo(shadowMapSize);
			renderingInfo.setLayers(6);
			renderingInfo.setViewMask(0b111111);
			
			renderingInfo.setDepthAttachment(*pointLightRenderData.shadowMapTextureView)
				.setLoadOpClear(1.0f)
				.setStoreOpStore();
			
			commandBuffer->pushDebugGroup(std::format("Point light ({})", shadowCastingPointLightIndex));
			
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
			
			PointLightUniforms uniforms{};
			std::copy(std::begin(pointLightRenderData.viewProjections), std::end(pointLightRenderData.viewProjections), std::begin(uniforms.viewProjections));
			uniforms.lightPos = pointLightRenderData.pos;
			uniforms.far = pointLightRenderData.far;
			
			std::memcpy(pointLightUniformBufferPtr, &uniforms, sizeof(PointLightUniforms));
			pointLightUniformBufferPtr++;
			
			commandBuffer->pushDescriptor(0, 0, _pointLightUniformBuffer.getCurrent()->getBuffer(), shadowCastingPointLightIndex, 1);
			
			for (const ModelRenderer::RenderData& modelRendererRenderData : input.registry.getModelRenderRequests())
			{
				if (!modelRendererRenderData.contributeShadows)
				{
					continue;
				}
				
				const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = modelRendererRenderData.mesh->getPositionVertexBuffer();
				const VKPtr<VKBuffer<uint32_t>>& indexBuffer = modelRendererRenderData.mesh->getIndexBuffer();
				
				commandBuffer->bindVertexBuffer(0, vertexBuffer);
				commandBuffer->bindIndexBuffer(indexBuffer);
				
				PointLightPushConstantData pushConstantData{};
				pushConstantData.model = modelRendererRenderData.matrix;
				commandBuffer->pushConstants(pushConstantData);
				
				commandBuffer->draw(indexBuffer->getSize(), 0);
			}
			
			commandBuffer->unbindPipeline();
			
			commandBuffer->endRendering();
			
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