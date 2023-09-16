#include "ZPrepass.h"

#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/VertexData.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"

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
		_depthImage,
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	VKRenderingInfo renderingInfo(_size);
	
	renderingInfo.setDepthAttachment(_depthImageView)
		.setLoadOpClear(1.0f)
		.setStoreOpStore();
	
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
	
	for (const ModelRenderer::RenderData& model : input.registry.getModelRenderRequests())
	{
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh.getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		PushConstantData pushConstantData{};
		pushConstantData.mvp = vp * model.transform.getLocalToWorldMatrix();
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {
		.multisampledDepthImageView = _depthImageView
	};
}

void ZPrepass::onResize()
{
	createImage();
}

void ZPrepass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.setPushConstantLayout<PushConstantData>();
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ZPrepass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/z-prepass/z-prepass.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
	
	info.setRasterizationSampleCount(vk::SampleCountFlagBits::e4);
	
	info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eLess, true);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ZPrepass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::DEPTH_FORMAT,
		_size,
		1,
		1,
		vk::ImageUsageFlagBits::eDepthStencilAttachment);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setSampleCount(vk::SampleCountFlagBits::e4);
	
	_depthImage = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKImageViewInfo imageViewInfo(
		_depthImage,
		vk::ImageViewType::e2D);
	
	_depthImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
}