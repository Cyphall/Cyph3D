#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"

namespace c3d
{
class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;

struct BloomPassInput
{
	const std::shared_ptr<VKImage>& inputImage;
};

struct BloomPassOutput
{
	const std::shared_ptr<VKImage>& outputImage;
};

class BloomPass : public RenderPass<BloomPassInput, BloomPassOutput>
{
public:
	explicit BloomPass(glm::uvec2 size);

private:
	// common

	std::shared_ptr<VKImage> _workImage;
	std::shared_ptr<VKImage> _outputImage;

	std::shared_ptr<VKSampler> _downsampleSampler;
	std::shared_ptr<VKSampler> _upsampleSampler;

	// downsample

	struct DownsamplePushConstantData
	{
		glm::vec2 srcPixelSize;
		int32_t srcLevel;
	};

	std::shared_ptr<VKDescriptorSetLayout> _downsampleDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _downsamplePipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _downsamplePipeline;

	// upsample

	struct UpsamplePushConstantData
	{
		glm::vec2 srcPixelSize;
		int32_t srcLevel;
		float bloomRadius;
	};

	std::shared_ptr<VKDescriptorSetLayout> _upsampleDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _upsamplePipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _upsamplePipeline;

	// compose

	struct ComposePushConstantData
	{
		float factor;
	};

	std::shared_ptr<VKDescriptorSetLayout> _composeDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _composePipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _composePipeline;

	std::shared_ptr<VKSampler> _inputImageSampler;


	void downsampleAnsBlur(const std::shared_ptr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void upsampleAndBlur(const std::shared_ptr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void compose(const std::shared_ptr<VKImage>& input, const std::shared_ptr<VKCommandBuffer>& commandBuffer);

	void createDescriptorSetLayouts();
	void createPipelineLayouts();
	void createPipelines();
	void createImages();
	void createSamplers();

	BloomPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, BloomPassInput& input) override;
	void onResize() override;
};
}