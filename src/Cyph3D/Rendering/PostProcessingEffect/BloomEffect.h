#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;
class VKImageView;

class BloomEffect : public PostProcessingEffect
{
public:
	explicit BloomEffect(glm::uvec2 size);

private:
	
	// common
	
	VKDynamic<VKImage> _workImage;
	std::vector<VKDynamic<VKImageView>> _workImageViews;
	
	VKDynamic<VKImage> _outputImage;
	VKDynamic<VKImageView> _outputImageView;
	
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
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera) override;
	void onResize() override;
};