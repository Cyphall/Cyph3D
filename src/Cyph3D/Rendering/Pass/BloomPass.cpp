#include "BloomPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

#include <format>

static const float BLOOM_RADIUS = 0.85f;
static const float BLOOM_STRENGTH = 0.15f;

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
			input.inputImage,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal);

		commandBuffer->imageMemoryBarrier(
			_workImage,
			{0, 0},
			{0, 0},
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferWrite,
			vk::ImageLayout::eTransferDstOptimal);

		commandBuffer->copyImageToImage(input.inputImage, 0, 0, _workImage, 0, 0);

		commandBuffer->imageMemoryBarrier(
			_workImage,
			{0, 0},
			{0, 0},
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
		commandBuffer->pushDebugGroup(std::format("downsampleAnsBlur({}->{})", i-1, i));
		{
			commandBuffer->imageMemoryBarrier(
				_workImage,
				{0, 0},
				{i, i},
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eColorAttachmentOptimal);

			downsampleAnsBlur(commandBuffer, i);

			commandBuffer->imageMemoryBarrier(
				_workImage,
				{0, 0},
				{i, i},
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
				_workImage,
				{0, 0},
				{i, i},
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentRead,
				vk::ImageLayout::eColorAttachmentOptimal);

			upsampleAndBlur(commandBuffer, i);

			commandBuffer->imageMemoryBarrier(
				_workImage,
				{0, 0},
				{i, i},
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
			input.inputImage,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::PipelineStageFlagBits2::eFragmentShader,
			vk::AccessFlagBits2::eShaderSampledRead,
			vk::ImageLayout::eReadOnlyOptimal);

		commandBuffer->imageMemoryBarrier(
			_outputImage,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eColorAttachmentOptimal);

		compose(input.inputImage, commandBuffer);
	}
	commandBuffer->popDebugGroup();

	return {
		_outputImage
	};
}

void BloomPass::onResize()
{
	createImages();
}

void BloomPass::downsampleAnsBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel)
{
	VKRenderingInfo renderingInfo(_workImage->getSize(dstLevel));

	renderingInfo.addColorAttachment(
		_workImage,
		vk::ImageViewType::e2D,
		{0, 0},
		{dstLevel, dstLevel},
		_workImage->getInfo().getFormat())
		.setLoadOpDontCare()
		.setStoreOpStore();

	commandBuffer->beginRendering(renderingInfo);

	commandBuffer->bindPipeline(_downsamplePipeline);

	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = _workImage->getSize(dstLevel);
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);

	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = _workImage->getSize(dstLevel);
	commandBuffer->setScissor(scissor);

	commandBuffer->pushDescriptor(
		0,
		0,
		_workImage,
		vk::ImageViewType::e2D,
		{0, 0},
		{dstLevel-1, dstLevel-1},
		_workImage->getInfo().getFormat(),
		_downsampleSampler);

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
	VKRenderingInfo renderingInfo(_workImage->getSize(dstLevel));

	renderingInfo.addColorAttachment(
			_workImage,
			vk::ImageViewType::e2D,
			{0, 0},
			{dstLevel, dstLevel},
			_workImage->getInfo().getFormat())
		.setLoadOpLoad()
		.setStoreOpStore();

	commandBuffer->beginRendering(renderingInfo);

	commandBuffer->bindPipeline(_upsamplePipeline);

	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = _workImage->getSize(dstLevel);
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);

	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = _workImage->getSize(dstLevel);
	commandBuffer->setScissor(scissor);

	commandBuffer->pushDescriptor(
		0,
		0,
		_workImage,
		vk::ImageViewType::e2D,
		{0, 0},
		{dstLevel+1, dstLevel+1},
		_workImage->getInfo().getFormat(),
		_upsampleSampler);

	UpsamplePushConstantData pushConstantData{};
	pushConstantData.srcPixelSize = glm::vec2(1.0f) / glm::vec2(_workImage->getSize(dstLevel+1));
	pushConstantData.srcLevel = dstLevel+1;
	pushConstantData.bloomRadius = glm::clamp(BLOOM_RADIUS, 0.0f, 1.0f);
	commandBuffer->pushConstants(pushConstantData);

	commandBuffer->draw(3, 0);

	commandBuffer->unbindPipeline();

	commandBuffer->endRendering();
}

void BloomPass::compose(const VKPtr<VKImage>& input, const VKPtr<VKCommandBuffer>& commandBuffer)
{
	VKRenderingInfo renderingInfo(_size);

	renderingInfo.addColorAttachment(_outputImage)
		.setLoadOpDontCare()
		.setStoreOpStore();

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
	commandBuffer->pushDescriptor(0, 1, _workImage, _inputImageSampler);

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
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setName("Bloom work image");

		_workImage = VKImage::create(Engine::getVKContext(), imageInfo);
	}

	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setName("Bloom output image");

		_outputImage = VKImage::create(Engine::getVKContext(), imageInfo);
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

		_downsampleSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}

	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;

		_upsampleSampler = VKSampler::create(Engine::getVKContext(), createInfo);
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