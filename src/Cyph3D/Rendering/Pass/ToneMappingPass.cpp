#include "ToneMappingPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

const vk::Format LINEAR_OUTPUT_FORMAT = vk::Format::eR8G8B8A8Unorm;
const vk::Format SRGB_OUTPUT_FORMAT = vk::Format::eR8G8B8A8Srgb;

ToneMappingPass::ToneMappingPass(glm::uvec2 size):
	RenderPass(size, "ToneMapping pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createSampler();
	createImage();
}

ToneMappingPassOutput ToneMappingPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ToneMappingPassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_outputImage.getCurrent(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal,
		0,
		0);
	
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = _outputSrgbImageView->getHandle();
	colorAttachment.imageLayout = _outputImage->getLayout(0, 0);
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	colorAttachment.resolveImageView = nullptr;
	colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(_size.x, _size.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = nullptr;
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
	
	commandBuffer->pushDescriptor(0, 0, input.inputImageView, _inputSampler);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {
		_outputLinearImageView.getCurrent()
	};
}

void ToneMappingPass::onResize()
{
	createImage();
}

void ToneMappingPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ToneMappingPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_descriptorSetLayout);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ToneMappingPass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/fullscreen quad.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.setFragmentShader("resources/shaders/internal/post-processing/tone mapping/tone mapping.frag");
	
	info.getPipelineAttachmentInfo().addColorAttachment(SRGB_OUTPUT_FORMAT);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ToneMappingPass::createSampler()
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

void ToneMappingPass::createImage()
{
	VKImageInfo imageInfo(
		SRGB_OUTPUT_FORMAT,
		_size,
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.addAdditionalCompatibleViewFormat(LINEAR_OUTPUT_FORMAT);
	
	_outputImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKImage::create(context, imageInfo);
	});
	
	_outputLinearImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		VKImageViewInfo imageViewInfo(
			_outputImage[index],
			vk::ImageViewType::e2D);
		imageViewInfo.setCustomViewFormat(LINEAR_OUTPUT_FORMAT);
		
		return VKImageView::create(context, imageViewInfo);
	});
	
	_outputSrgbImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		VKImageViewInfo imageViewInfo(
			_outputImage[index],
			vk::ImageViewType::e2D);
		imageViewInfo.setCustomViewFormat(SRGB_OUTPUT_FORMAT);
		
		return VKImageView::create(context, imageViewInfo);
	});
}