#include "NormalizationPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

NormalizationPass::NormalizationPass(glm::uvec2 size):
	RenderPass(size, "Normalization pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createSampler();
	createImage();
}

NormalizationPassOutput NormalizationPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_outputImage,
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	VKRenderingInfo renderingInfo(_size);
	
	renderingInfo.addColorAttachment(_outputImageView)
		.setLoadOpDontCare()
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
	
	commandBuffer->pushDescriptor(0, 0, input.inputImageView, _inputSampler);
	
	PushConstantData pushConstantData{};
	pushConstantData.accumulatedBatches = input.accumulatedBatches;
	commandBuffer->pushConstants(pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {
		_outputImageView
	};
}

void NormalizationPass::onResize()
{
	createImage();
}

void NormalizationPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void NormalizationPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void NormalizationPass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/fullscreen quad.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.setFragmentShader("resources/shaders/internal/post-processing/normalization/normalization.frag");
	
	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void NormalizationPass::createSampler()
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
	createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	createInfo.unnormalizedCoordinates = false;
	
	_inputSampler = VKSampler::create(Engine::getVKContext(), createInfo);
}

void NormalizationPass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::HDR_COLOR_FORMAT,
		_size,
		1,
		1,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_outputImage = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKImageViewInfo imageViewInfo(
		_outputImage,
		vk::ImageViewType::e2D);
	
	_outputImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
}