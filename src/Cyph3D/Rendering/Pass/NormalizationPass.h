#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKComputePipeline;
class VKSampler;
class VKImage;
class VKImageView;

struct NormalizationPassInput
{
	std::array<VKPtr<VKImageView>, 3> inputImageView;
	uint32_t accumulatedSamples;
	uint32_t fixedPointDecimals;
};

struct NormalizationPassOutput
{
	const VKPtr<VKImageView>& outputImageView;
};

class NormalizationPass : public RenderPass<NormalizationPassInput, NormalizationPassOutput>
{
public:
	explicit NormalizationPass(glm::uvec2 size);

private:
	struct PushConstantData
	{
		GLSL_uint accumulatedSamples;
		GLSL_uint fixedPointDecimals;
	};
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKComputePipeline> _pipeline;
	
	VKPtr<VKImage> _outputImage;
	VKPtr<VKImageView> _outputImageView;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();
	
	NormalizationPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input) override;
	void onResize() override;
};