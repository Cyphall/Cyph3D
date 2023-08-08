#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;
class VKImageView;

struct NormalizationPassInput
{
	const VKPtr<VKImageView>& inputImageView;
	uint32_t accumulatedBatches;
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
		GLSL_uint accumulatedBatches;
	};
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKSampler> _inputSampler;
	
	VKPtr<VKImage> _outputImage;
	VKPtr<VKImageView> _outputImageView;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createSampler();
	void createImage();
	
	NormalizationPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input) override;
	void onResize() override;
};