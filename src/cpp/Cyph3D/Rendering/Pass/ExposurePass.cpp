#include "ExposurePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

ExposurePass::ExposurePass(glm::uvec2 size):
	RenderPass(size, "Exposure pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createSampler();
	createImage();
}

ExposurePassOutput ExposurePass::onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, ExposurePassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		input.inputImage,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	commandBuffer->imageMemoryBarrier(
		_outputImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	VKRenderingInfo renderingInfo(_size);

	renderingInfo.addColorAttachment(_outputImage)
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

	commandBuffer->pushDescriptor(0, 0, input.inputImage, _inputSampler);

	PushConstantData pushConstantData{};
	pushConstantData.exposure = input.camera.getExposure();
	commandBuffer->pushConstants(pushConstantData);

	commandBuffer->draw(3, 0);

	commandBuffer->unbindPipeline();

	commandBuffer->endRendering();

	return {
		_outputImage
	};
}

void ExposurePass::onResize()
{
	createImage();
}

void ExposurePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);

	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ExposurePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();

	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ExposurePass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"fullscreen quad.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise
	);

	info.setFragmentShader("post-processing/exposure/exposure.frag");

	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);

	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ExposurePass::createSampler()
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

void ExposurePass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::HDR_COLOR_FORMAT,
		_size,
		1,
		1,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setName("Exposure output image");

	_outputImage = VKImage::create(Engine::getVKContext(), imageInfo);
}