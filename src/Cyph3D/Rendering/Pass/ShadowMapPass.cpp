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
	for (DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		if (!renderData.castShadows)
		{
			continue;
		}
		
		commandBuffer->imageMemoryBarrier(
			(*renderData.shadowMapTextureView)->getInfo().getImage(),
			0,
			0,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::ImageLayout::eDepthAttachmentOptimal);
		
		glm::uvec2 shadowMapSize = (*renderData.shadowMapTextureView)->getInfo().getImage()->getSize(0);
		
		VKRenderingInfo renderingInfo(shadowMapSize);
		
		renderingInfo.setDepthAttachment(renderData.shadowMapTextureView->getCurrent())
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
		
		for (const ModelRenderer::RenderData& modelData : input.registry.models)
		{
			if (!modelData.contributeShadows)
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
			
			DirectionalLightPushConstantData pushConstantData{};
			pushConstantData.mvp = renderData.lightViewProjection * modelData.matrix;
			commandBuffer->pushConstants(pushConstantData);
			
			commandBuffer->draw(indexBuffer->getSize(), 0);
		}
		
		commandBuffer->unbindPipeline();
		
		commandBuffer->endRendering();
	}
	
	int shadowCastingPointLights = 0;
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		if (renderData.castShadows)
		{
			shadowCastingPointLights++;
		}
	}
	
	int shadowCastingPointLightIndex = 0;
	_pointLightUniformBuffer->resizeSmart(shadowCastingPointLights);
	PointLightUniforms* pointLightUniformBufferPtr = _pointLightUniformBuffer->getHostPointer();
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		if (!renderData.castShadows)
		{
			continue;
		}
		
		for (int i = 0; i < 6; i++)
		{
			commandBuffer->imageMemoryBarrier(
				renderData.shadowMapTexture->getCurrent(),
				i,
				0,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentRead,
				vk::ImageLayout::eDepthAttachmentOptimal);
		}
		
		glm::uvec2 shadowMapSize = (*renderData.shadowMapTexture)->getSize(0);
		
		VKRenderingInfo renderingInfo(shadowMapSize);
		renderingInfo.setLayers(6);
		
		renderingInfo.setDepthAttachment(renderData.shadowMapTextureView->getCurrent())
			.setLoadOpClear(1.0f)
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
		
		PointLightUniforms uniforms{};
		std::copy(std::begin(renderData.viewProjections), std::end(renderData.viewProjections), std::begin(uniforms.viewProjections));
		uniforms.lightPos = renderData.pos;
		uniforms.far = renderData.far;
		
		std::memcpy(pointLightUniformBufferPtr + shadowCastingPointLightIndex, &uniforms, sizeof(PointLightUniforms));
		
		commandBuffer->pushDescriptor(0, 0, _pointLightUniformBuffer.getCurrent()->getBuffer(), shadowCastingPointLightIndex, 1);
		
		for (const ModelRenderer::RenderData& modelData : input.registry.models)
		{
			if (!modelData.contributeShadows)
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
			
			PointLightPushConstantData pushConstantData{};
			pushConstantData.model = modelData.matrix;
			commandBuffer->pushConstants(pushConstantData);
			
			commandBuffer->draw(indexBuffer->getSize(), 0);
		}
		
		commandBuffer->unbindPipeline();
		
		commandBuffer->endRendering();
		
		shadowCastingPointLightIndex++;
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
	_pointLightUniformBuffer = VKDynamic<VKResizableBuffer<PointLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<PointLightUniforms>::create(
			context,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
		
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
		
		info.setGeometryShader("resources/shaders/internal/shadow mapping/point light.geom");
		info.setFragmentShader("resources/shaders/internal/shadow mapping/point light.frag");
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(PointLight::depthFormat, vk::CompareOp::eLess, true);
		
		_pointLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}