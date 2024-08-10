#include "NormalizationPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKComputePipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"

NormalizationPass::NormalizationPass(glm::uvec2 size):
	RenderPass(size, "Normalization pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImage();
}

NormalizationPassOutput NormalizationPass::onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input)
{
	for (int i = 0; i < 3; i++)
	{
		commandBuffer->imageMemoryBarrier(
			input.inputImage[i],
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral
		);
	}

	commandBuffer->imageMemoryBarrier(
		_outputImage,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral
	);

	VKRenderingInfo renderingInfo(_size);

	commandBuffer->bindPipeline(_pipeline);

	for (int i = 0; i < 3; i++)
	{
		commandBuffer->pushDescriptor(0, 0, input.inputImage[i], i);
	}
	commandBuffer->pushDescriptor(0, 1, _outputImage, 0);

	PushConstantData pushConstantData{
		.accumulatedSamples = input.accumulatedSamples
	};
	commandBuffer->pushConstants(pushConstantData);

	commandBuffer->dispatch({(_size.x + 7) / 8, (_size.y + 7) / 8, 1});

	commandBuffer->unbindPipeline();

	return {
		_outputImage
	};
}

void NormalizationPass::onResize()
{
	createImage();
}

void NormalizationPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eStorageImage, 3);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);

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
	VKComputePipelineInfo info(
		_pipelineLayout,
		FileHelper::getDataDirectory() / "shaders/post-processing/normalization/normalization.comp"
	);

	_pipeline = VKComputePipeline::create(Engine::getVKContext(), info);
}

void NormalizationPass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::HDR_COLOR_FORMAT,
		_size,
		1,
		1,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setName("Normalization output image");

	_outputImage = VKImage::create(Engine::getVKContext(), imageInfo);
}