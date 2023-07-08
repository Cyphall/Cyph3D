#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;
class VKImageView;

struct BloomPassInput
{
	const VKPtr<VKImageView>& inputImageView;
};

struct BloomPassOutput
{
	const VKPtr<VKImageView>& outputImageView;
};

class BloomPass : public RenderPass<BloomPassInput, BloomPassOutput>
{
public:
	explicit BloomPass(glm::uvec2 size);

private:
	
	// common
	
	VKPtr<VKImage> _workImage;
	std::vector<VKPtr<VKImageView>> _workImageViews;
	
	VKPtr<VKImage> _outputImage;
	VKPtr<VKImageView> _outputImageView;
	
	VKPtr<VKSampler> _workImageSampler;
	
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

	
	void downsample(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void upsampleAndBlur(const VKPtr<VKCommandBuffer>& commandBuffer, int dstLevel);
	void compose(const VKPtr<VKImageView>& input, const VKPtr<VKCommandBuffer>& commandBuffer);
	
	void createDescriptorSetLayouts();
	void createPipelineLayouts();
	void createPipelines();
	void createImages();
	void createSamplers();
	
	BloomPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, BloomPassInput& input) override;
	void onResize() override;
};