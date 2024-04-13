#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKComputePipeline;
class VKSampler;
class VKImage;

struct NormalizationPassInput
{
	std::array<VKPtr<VKImage>, 3> inputImage;
	uint32_t accumulatedSamples;
};

struct NormalizationPassOutput
{
	const VKPtr<VKImage>& outputImage;
};

class NormalizationPass : public RenderPass<NormalizationPassInput, NormalizationPassOutput>
{
public:
	explicit NormalizationPass(glm::uvec2 size);

private:
	struct PushConstantData
	{
		GLSL_uint accumulatedSamples;
	};

	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;

	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKComputePipeline> _pipeline;

	VKPtr<VKImage> _outputImage;

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();

	NormalizationPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input) override;
	void onResize() override;
};