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
	createImage();
}

ZPrepassOutput ZPrepass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ZPrepassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_depthImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal,
		0,
		0);
	
	vk::RenderingAttachmentInfo depthAttachment;
	depthAttachment.imageView = _depthImageView->getHandle();
	depthAttachment.imageLayout = _depthImage->getLayout(0, 0);
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
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = _size;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = _size;
	commandBuffer->setScissor(scissor);
	
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
		.depthImageView = _depthImageView.getVKPtr()
	};
}

void ZPrepass::onResize()
{
	createImage();
}

void ZPrepass::createPipelineLayout()
{
	VKPipelineLayoutInfo pipelineLayoutInfo;
	pipelineLayoutInfo.registerPushConstantLayout<PushConstantData>(vk::ShaderStageFlagBits::eVertex);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), pipelineLayoutInfo);
}

void ZPrepass::createPipeline()
{
	VKGraphicsPipelineInfo info;
	info.vertexShaderFile = "resources/shaders/internal/z-prepass/z-prepass.vert";
	info.geometryShaderFile = std::nullopt;
	info.fragmentShaderFile = std::nullopt;
	
	info.vertexInputLayoutInfo.defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	info.vertexInputLayoutInfo.defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
	
	info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
	
	info.pipelineLayout = _pipelineLayout;
	
	info.viewport = std::nullopt;
	
	info.scissor = std::nullopt;
	
	info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
	info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	
	info.pipelineAttachmentInfo.setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eLess, true);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ZPrepass::createImage()
{
	_depthImage = VKImage::createDynamic(
		Engine::getVKContext(),
		SceneRenderer::DEPTH_FORMAT,
		_size,
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::ImageAspectFlagBits::eDepth,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_depthImageView = VKImageView::createDynamic(
		Engine::getVKContext(),
		_depthImage,
		vk::ImageViewType::e2D);
}