#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;

struct BloomPassInput
{
	const VKPtr<VKImage>& inputImage;
};

struct BloomPassOutput
{
	const VKPtr<VKImage>& outputImage;
};

class BloomPass : public RenderPass<BloomPassInput, BloomPassOutput>
{
public:
	explicit BloomPass(glm::uvec2 size);

private:

	// common

	VKPtr<VKImage> _workImage;
	VKPtr<VKImage> _outputImage;

	VKPtr<VKSampler> _downsampleSampler;
	VKPtr<VKSampler> _upsampleSampler;

	// downsample

	struct DownsamplePushConstantData
	{
		GLSL_vec2 srcPixelSize;
		GLSL_int srcLevel;
	};

	VKPtr<VKDescriptorSetLayout> _downsampleDescriptorSetLayout;

	VKPtr<VKPipelineLayout> _downsamplePipelineLayout;
	VKPtr<VKGraphicsPipeline> _downsamplePipeline;

	// upsample

	struct UpsamplePushConstantData
	{
		GLSL_vec2 srcPixelSize;
		GLSL_int srcLevel;
		GLSL_float bloomRadius;
	};

	VKPtr<VKDescriptorSetLayout> _upsampleDescriptorSetLayout;

	VKPtr<VKPipelineLayout> _upsamplePipelineLayout;
	VKPtr<VKGraphicsPipeline> _upsamplePipeline;

	// compose

	struct ComposePushConstantData
	{
		GLSL_float factor;
	};

	VKPtr<VKDescriptorSetLayout> _composeDescriptorSetLayout;

	VKPtr<VKPipelineLayout> _composePipelineLayout;
	VKPtr<VKGraphicsPipeline> _composePipeline;

	VKPtr<VKSampler> _inputImageSampler;


	void downsampleAnsBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void upsampleAndBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void compose(const VKPtr<VKImage>& input, const VKPtr<VKCommandBuffer>& commandBuffer);

	void createDescriptorSetLayouts();
	void createPipelineLayouts();
	void createPipelines();
	void createImages();
	void createSamplers();

	BloomPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, BloomPassInput& input) override;
	void onResize() override;
};