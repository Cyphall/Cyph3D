#include "BloomPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

#include <format>

static const float BLOOM_RADIUS = 0.85f;
static const float BLOOM_STRENGTH = 0.3f;

BloomPass::BloomPass(glm::uvec2 size):
	RenderPass(size, "Bloom pass")
{
	createDescriptorSetLayouts();
	createPipelineLayouts();
	createPipelines();
	createImages();
	createSamplers();
}

BloomPassOutput BloomPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, BloomPassInput& input)
{
	// copy inputImageView image level 0 to work image level 0
	commandBuffer->pushDebugGroup("copyImageBaseLevel");
	{
		commandBuffer->imageMemoryBarrier(
			input.inputImageView->getInfo().getImage(),
			0,
			0,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal);
		
		commandBuffer->imageMemoryBarrier(
			_workImage.getCurrent(),
			0,
			0,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal);
		
		commandBuffer->copyImageToImage(input.inputImageView->getInfo().getImage(), 0, 0, _workImage.getCurrent(), 0, 0);
		
		commandBuffer->imageMemoryBarrier(
			_workImage.getCurrent(),
			0,
			0,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal);
	}
	commandBuffer->popDebugGroup();

	// downsample work image
	for (int i = 1; i < _workImage->getInfo().getLevels(); i++)
	{
		commandBuffer->pushDebugGroup(std::format("downsample({}->{})", i-1, i));
		{
			commandBuffer->imageMemoryBarrier(
				_workImage.getCurrent(),
				0,
				i,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eColorAttachmentOptimal);
			
			downsample(commandBuffer, i);
			
			commandBuffer->imageMemoryBarrier(
				_workImage.getCurrent(),
				0,
				i,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal);
		}
		commandBuffer->popDebugGroup();
	}

	// upsample and blur work image
	for (int i = _workImage->getInfo().getLevels() - 2; i >= 0; i--)
	{
		commandBuffer->pushDebugGroup(std::format("upsampleAndBlur({}->{})", i+1, i));
		{
			commandBuffer->imageMemoryBarrier(
				_workImage.getCurrent(),
				0,
				i,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentRead,
				vk::ImageLayout::eColorAttachmentOptimal);
			
			upsampleAndBlur(commandBuffer, i);
			
			commandBuffer->imageMemoryBarrier(
				_workImage.getCurrent(),
				0,
				i,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal);
		}
		commandBuffer->popDebugGroup();
	}

	// compose inputImageView image level 0 and work image level 0 to outputImageView image level 0
	commandBuffer->pushDebugGroup("compose");
	{
		commandBuffer->imageMemoryBarrier(
			input.inputImageView->getInfo().getImage(),
			0,
			0,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal);
		
		commandBuffer->imageMemoryBarrier(
			_outputImage.getCurrent(),
			0,
			0,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eColorAttachmentOptimal);
		
		compose(input.inputImageView, commandBuffer);
	}
	commandBuffer->popDebugGroup();

	return {
		_outputImageView.getCurrent()
	};
}

void BloomPass::onResize()
{
	_workImageViews.clear();
	createImages();
}

void BloomPass::downsample(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel)
{
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = _workImageViews[dstLevel]->getHandle();
	colorAttachment.imageLayout = _workImage->getLayout(0, dstLevel);
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	colorAttachment.resolveImageView = nullptr;
	colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	glm::uvec2 viewportSize = _workImage->getSize(dstLevel);
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(viewportSize.x, viewportSize.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_downsamplePipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = viewportSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = viewportSize;
	commandBuffer->setScissor(scissor);
	
	commandBuffer->pushDescriptor(0, 0, _workImageViews[dstLevel-1].getCurrent(), _workImageSampler);
	
	DownsamplePushConstantData pushConstantData{};
	pushConstantData.srcPixelSize = glm::vec2(1.0f) / glm::vec2(_workImage->getSize(dstLevel-1));
	pushConstantData.srcLevel = dstLevel-1;
	commandBuffer->pushConstants(pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomPass::upsampleAndBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel)
{
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = _workImageViews[dstLevel]->getHandle();
	colorAttachment.imageLayout = _workImage->getLayout(0, dstLevel);
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	colorAttachment.resolveImageView = nullptr;
	colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	glm::uvec2 viewportSize = _workImage->getSize(dstLevel);
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(viewportSize.x, viewportSize.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_upsamplePipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = viewportSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = viewportSize;
	commandBuffer->setScissor(scissor);
	
	commandBuffer->pushDescriptor(0, 0, _workImageViews[dstLevel+1].getCurrent(), _workImageSampler);
	
	UpsamplePushConstantData pushConstantData{};
	pushConstantData.srcPixelSize = glm::vec2(1.0f) / glm::vec2(_workImage->getSize(dstLevel+1));
	pushConstantData.srcLevel = dstLevel+1;
	pushConstantData.bloomRadius = glm::clamp(BLOOM_RADIUS, 0.0f, 1.0f);
	commandBuffer->pushConstants(pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomPass::compose(const VKPtr<VKImageView>& input, const VKPtr<VKCommandBuffer>& commandBuffer)
{
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = _outputImageView->getHandle();
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
	
	commandBuffer->bindPipeline(_composePipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = _size;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = _size;
	commandBuffer->setScissor(scissor);
	
	commandBuffer->pushDescriptor(0, 0, input, _inputImageSampler);
	commandBuffer->pushDescriptor(0, 1, _workImageViews[0].getCurrent(), _workImageSampler);
	
	ComposePushConstantData pushConstantData{};
	pushConstantData.factor = glm::clamp(BLOOM_STRENGTH, 0.0f, 1.0f);
	commandBuffer->pushConstants(pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomPass::createDescriptorSetLayouts()
{
	{
		VKDescriptorSetLayoutInfo info(true);
		info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
		
		_downsampleDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(true);
		info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
		
		_upsampleDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(true);
		info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
		info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
		
		_composeDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
}

void BloomPass::createPipelineLayouts()
{
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_downsampleDescriptorSetLayout);
		info.setPushConstantLayout<DownsamplePushConstantData>();
		
		_downsamplePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_upsampleDescriptorSetLayout);
		info.setPushConstantLayout<UpsamplePushConstantData>();
		
		_upsamplePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_composeDescriptorSetLayout);
		info.setPushConstantLayout<ComposePushConstantData>();
		
		_composePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
}

void BloomPass::createPipelines()
{
	{
		VKGraphicsPipelineInfo info(
			_downsamplePipelineLayout,
			"resources/shaders/internal/fullscreen quad.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/post-processing/bloom/downsample.frag");
		
		info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
		
		_downsamplePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info(
			_upsamplePipelineLayout,
			"resources/shaders/internal/fullscreen quad.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/post-processing/bloom/upsample and blur.frag");
		
		VKPipelineBlendingInfo blendingInfo{
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.alphaBlendOp = vk::BlendOp::eAdd
		};
		
		info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT, blendingInfo);
		
		_upsamplePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info(
			_composePipelineLayout,
			"resources/shaders/internal/fullscreen quad.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/post-processing/bloom/compose.frag");
		
		info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
		
		_composePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}

void BloomPass::createImages()
{
	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			VKImage::calcMaxMipLevels(_size),
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_workImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		for (int i = 0; i < _workImage->getInfo().getLevels(); i++)
		{
			_workImageViews.emplace_back(Engine::getVKContext(), [&](VKContext& context, int index)
			{
				VKImageViewInfo imageViewInfo(
					_workImage[index],
					vk::ImageViewType::e2D);
				imageViewInfo.setCustomLevelRange({i, i});
				
				return VKImageView::create(context, imageViewInfo);
			});
		}
	}
	
	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_outputImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_outputImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_outputImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
}

void BloomPass::createSamplers()
{
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
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
		
		_workImageSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
	
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
		
		_inputImageSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
}