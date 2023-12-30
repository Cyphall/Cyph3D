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
#include "Cyph3D/VKObject/Image/VKImage.h"
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
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal
	);

	VKRenderingInfo renderingInfo(_size);

	renderingInfo.setDepthAttachment(_depthImage)
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

	commandBuffer->pushConstants(PushConstantData{
		.viewProjection = input.camera.getProjection() * input.camera.getView(),
		.modelDataBuffer = input.modelDataBuffer ? input.modelDataBuffer->getDeviceAddress() : 0
	});

	commandBuffer->addExternallyUsedObject(input.modelDataBuffer);

	commandBuffer->drawIndirect(input.drawCommandsBuffer);

	commandBuffer->unbindPipeline();

	commandBuffer->endRendering();

	return {
		.multisampledDepthImage = _depthImage
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
		vk::FrontFace::eCounterClockwise
	);

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
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setSampleCount(vk::SampleCountFlagBits::e4);
	imageInfo.setName("Depth image");

	_depthImage = VKImage::create(Engine::getVKContext(), imageInfo);
}