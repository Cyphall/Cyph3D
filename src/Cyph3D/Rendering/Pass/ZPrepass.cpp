#include "ZPrepass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/Shape.h"
#include "Cyph3D/Rendering/VertexData.h"
#include "Cyph3D/Scene/Camera.h"

ZPrepass::ZPrepass(glm::uvec2 size):
	RenderPass(size, "Z prepass")
{
	createPipelineLayout();
	createPipeline();
	createDepthMap();
}

ZPrepassOutput ZPrepass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ZPrepassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_depthMap.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal,
		0,
		0);
	
	vk::RenderingAttachmentInfo depthAttachment;
	depthAttachment.imageView = _depthMapView->getHandle();
	depthAttachment.imageLayout = _depthMap->getLayout(0, 0);
	depthAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	depthAttachment.resolveImageView = nullptr;
	depthAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.clearValue.depthStencil.depth = 1.0f;
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(_size.x, _size.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 0;
	renderingInfo.pColorAttachments = nullptr;
	renderingInfo.pDepthAttachment = &depthAttachment;
	renderingInfo.pStencilAttachment = nullptr;
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_pipeline);
	
	glm::mat4 vp = input.camera.getProjection() * input.camera.getView();
	
	for (const ShapeRenderer::RenderData& shapeData : input.registry.shapes)
	{
		if (!shapeData.shape->isReadyForRasterisationRender())
			continue;
		
		const VKPtr<VKBuffer<VertexData>>& vertexBuffer = shapeData.shape->getVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = shapeData.shape->getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		PushConstantData pushConstantData{};
		pushConstantData.mvp = vp * shapeData.matrix;
		commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex, pushConstantData);
		
		commandBuffer->draw(indexBuffer->getSize(), 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {
		.depthView = _depthMapView.getVKPtr()
	};
}

void ZPrepass::createPipelineLayout()
{
	VKPipelineLayoutInfo pipelineLayoutInfo;
	pipelineLayoutInfo.registerPushConstantLayout<PushConstantData>(vk::ShaderStageFlagBits::eVertex);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), pipelineLayoutInfo);
}

void ZPrepass::createPipeline()
{
	VKGraphicsPipelineInfo graphicsPipelineInfo;
	graphicsPipelineInfo.vertexShaderFile = "resources/shaders/internal/z-prepass/z-prepass.vert";
	graphicsPipelineInfo.geometryShaderFile = std::nullopt;
	graphicsPipelineInfo.fragmentShaderFile = std::nullopt;
	
	graphicsPipelineInfo.vertexInputLayoutInfo.defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	graphicsPipelineInfo.vertexInputLayoutInfo.defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
	
	graphicsPipelineInfo.vertexTopology = vk::PrimitiveTopology::eTriangleList;
	
	graphicsPipelineInfo.pipelineLayout = _pipelineLayout;
	
	graphicsPipelineInfo.viewport = VKPipelineViewport{
		.offset = {0, 0},
		.size = _size,
		.depthRange = {0.0f, 1.0f}
	};
	
	graphicsPipelineInfo.scissor = VKPipelineScissor{
		.offset = graphicsPipelineInfo.viewport->offset,
		.size = graphicsPipelineInfo.viewport->size
	};
	
	graphicsPipelineInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
	graphicsPipelineInfo.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	
	graphicsPipelineInfo.pipelineAttachmentInfo.setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eLess, true);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), graphicsPipelineInfo);
}

void ZPrepass::createDepthMap()
{
	_depthMap = VKImage::createDynamic(
		Engine::getVKContext(),
		SceneRenderer::DEPTH_FORMAT,
		_size,
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::ImageAspectFlagBits::eDepth,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_depthMapView = VKImageView::createDynamic(
		Engine::getVKContext(),
		_depthMap,
		vk::ImageViewType::e2D);
}