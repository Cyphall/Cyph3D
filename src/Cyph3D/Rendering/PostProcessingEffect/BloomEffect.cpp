#include "BloomEffect.h"

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
static const float BLOOM_STRENGTH = 0.5f;

BloomEffect::BloomEffect(glm::uvec2 size):
	PostProcessingEffect("Bloom", size)
{
	createDescriptorSetLayouts();
	createPipelineLayouts();
	createPipelines();
	createImages();
	createSamplers();
}

const VKPtr<VKImageView>& BloomEffect::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera)
{
	// copy input image level 0 to work image level 0
	commandBuffer->pushDebugGroup("copyImageBaseLevel");
	{
		commandBuffer->imageMemoryBarrier(
			input->getImage(),
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal,
			0,
			0);
		
		commandBuffer->imageMemoryBarrier(
			_workImage.getVKPtr(),
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal,
			0,
			0);
		
		commandBuffer->copyImageToImage(input->getImage(), 0, 0, _workImage.getVKPtr(), 0, 0);
		
		commandBuffer->imageMemoryBarrier(
			_workImage.getVKPtr(),
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal,
			0,
			0);
	}
	commandBuffer->popDebugGroup();

	// downsample work image
	for (int i = 1; i < _workImage->getLevels(); i++)
	{
		commandBuffer->pushDebugGroup(std::format("downsample({}->{})", i-1, i));
		{
			commandBuffer->imageMemoryBarrier(
				_workImage.getVKPtr(),
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eColorAttachmentOptimal,
				0,
				i);
			
			downsample(commandBuffer, i);
			
			commandBuffer->imageMemoryBarrier(
				_workImage.getVKPtr(),
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal,
				0,
				i);
		}
		commandBuffer->popDebugGroup();
	}

	// upsample and blur work image
	for (int i = _workImage->getLevels() - 2; i >= 0; i--)
	{
		commandBuffer->pushDebugGroup(std::format("upsampleAndBlur({}->{})", i+1, i));
		{
			commandBuffer->imageMemoryBarrier(
				_workImage.getVKPtr(),
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentRead,
				vk::ImageLayout::eColorAttachmentOptimal,
				0,
				i);
			
			upsampleAndBlur(commandBuffer, i);
			
			commandBuffer->imageMemoryBarrier(
				_workImage.getVKPtr(),
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal,
				0,
				i);
		}
		commandBuffer->popDebugGroup();
	}

	// compose input image level 0 and work image level 0 to output image level 0
	commandBuffer->pushDebugGroup("compose");
	{
		commandBuffer->imageMemoryBarrier(
			input->getImage(),
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal,
			0,
			0);
		
		commandBuffer->imageMemoryBarrier(
			_outputImage.getVKPtr(),
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eColorAttachmentOptimal,
			0,
			0);
		
		compose(input, commandBuffer);
		
		commandBuffer->imageMemoryBarrier(
			_outputImage.getVKPtr(),
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal,
			0,
			0);
	}
	commandBuffer->popDebugGroup();

	return _outputImageView.getVKPtr();
}

void BloomEffect::onResize()
{
	_workImageViews.clear();
	createImages();
}

void BloomEffect::downsample(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel)
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
	
	commandBuffer->pushDescriptor(0, 0, _workImageViews[dstLevel-1].getVKPtr(), _workImageSampler);
	
	DownsamplePushConstantData pushConstantData{};
	pushConstantData.srcPixelSize = glm::vec2(1.0f) / glm::vec2(_workImage->getSize(dstLevel-1));
	pushConstantData.srcLevel = dstLevel-1;
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eFragment, pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomEffect::upsampleAndBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel)
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
	
	commandBuffer->pushDescriptor(0, 0, _workImageViews[dstLevel+1].getVKPtr(), _workImageSampler);
	
	UpsamplePushConstantData pushConstantData{};
	pushConstantData.srcPixelSize = glm::vec2(1.0f) / glm::vec2(_workImage->getSize(dstLevel+1));
	pushConstantData.srcLevel = dstLevel+1;
	pushConstantData.bloomRadius = glm::clamp(BLOOM_RADIUS, 0.0f, 1.0f);
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eFragment, pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomEffect::compose(const VKPtr<VKImageView>& input, const VKPtr<VKCommandBuffer>& commandBuffer)
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
	commandBuffer->pushDescriptor(0, 1, _workImageViews[0].getVKPtr(), _workImageSampler);
	
	ComposePushConstantData pushConstantData{};
	pushConstantData.factor = glm::clamp(BLOOM_STRENGTH, 0.0f, 1.0f);
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eFragment, pushConstantData);
	
	commandBuffer->draw(3, 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void BloomEffect::createDescriptorSetLayouts()
{
	{
		VKDescriptorSetLayoutInfo info(true);
		info.registerBinding(0, vk::DescriptorType::eCombinedImageSampler, 1);
		
		_downsampleDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(true);
		info.registerBinding(0, vk::DescriptorType::eCombinedImageSampler, 1);
		
		_upsampleDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKDescriptorSetLayoutInfo info(true);
		info.registerBinding(0, vk::DescriptorType::eCombinedImageSampler, 1);
		info.registerBinding(1, vk::DescriptorType::eCombinedImageSampler, 1);
		
		_composeDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}
}

void BloomEffect::createPipelineLayouts()
{
	{
		VKPipelineLayoutInfo info;
		info.registerDescriptorSetLayout(_downsampleDescriptorSetLayout);
		info.registerPushConstantLayout<DownsamplePushConstantData>(vk::ShaderStageFlagBits::eFragment);
		
		_downsamplePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.registerDescriptorSetLayout(_upsampleDescriptorSetLayout);
		info.registerPushConstantLayout<UpsamplePushConstantData>(vk::ShaderStageFlagBits::eFragment);
		
		_upsamplePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.registerDescriptorSetLayout(_composeDescriptorSetLayout);
		info.registerPushConstantLayout<ComposePushConstantData>(vk::ShaderStageFlagBits::eFragment);
		
		_composePipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
}

void BloomEffect::createPipelines()
{
	{
		VKGraphicsPipelineInfo info;
		info.vertexShaderFile = "resources/shaders/internal/fullscreen quad.vert";
		info.geometryShaderFile = std::nullopt;
		info.fragmentShaderFile = "resources/shaders/internal/post-processing/bloom/downsample.frag";
		
		info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
		
		info.pipelineLayout = _downsamplePipelineLayout;
		
		info.viewport = std::nullopt;
		
		info.scissor = std::nullopt;
		
		info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
		info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
		
		info.pipelineAttachmentInfo.registerColorAttachment(0, SceneRenderer::HDR_COLOR_FORMAT);
		
		_downsamplePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info;
		info.vertexShaderFile = "resources/shaders/internal/fullscreen quad.vert";
		info.geometryShaderFile = std::nullopt;
		info.fragmentShaderFile = "resources/shaders/internal/post-processing/bloom/upsample and blur.frag";
		
		info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
		
		info.pipelineLayout = _upsamplePipelineLayout;
		
		info.viewport = std::nullopt;
		
		info.scissor = std::nullopt;
		
		info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
		info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
		
		VKPipelineBlendingInfo blendingInfo{
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.alphaBlendOp = vk::BlendOp::eAdd
		};
		
		info.pipelineAttachmentInfo.registerColorAttachment(0, SceneRenderer::HDR_COLOR_FORMAT, blendingInfo);
		
		_upsamplePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info;
		info.vertexShaderFile = "resources/shaders/internal/fullscreen quad.vert";
		info.geometryShaderFile = std::nullopt;
		info.fragmentShaderFile = "resources/shaders/internal/post-processing/bloom/compose.frag";
		
		info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
		
		info.pipelineLayout = _composePipelineLayout;
		
		info.viewport = std::nullopt;
		
		info.scissor = std::nullopt;
		
		info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
		info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
		
		info.pipelineAttachmentInfo.registerColorAttachment(0, SceneRenderer::HDR_COLOR_FORMAT);
		
		_composePipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}

void BloomEffect::createImages()
{
	{
		_workImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			VKImage::calcMaxMipLevels(_size),
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	
	{
		for (int i = 0; i < _workImage->getLevels(); i++)
		{
			_workImageViews.emplace_back(VKImageView::createDynamic(
				Engine::getVKContext(),
				_workImage,
				vk::ImageViewType::e2D,
				std::nullopt,
				std::nullopt,
				std::nullopt,
				glm::vec2(i, i)));
		}
	}
	
	{
		_outputImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	}
	
	{
		_outputImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_outputImage,
			vk::ImageViewType::e2D);
	}
}

void BloomEffect::createSamplers()
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