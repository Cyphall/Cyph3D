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
	std::array<std::shared_ptr<VKImage>, 3> inputImage;
	uint32_t accumulatedSamples;
};

struct NormalizationPassOutput
{
	const std::shared_ptr<VKImage>& outputImage;
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

	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKComputePipeline> _pipeline;

	std::shared_ptr<VKImage> _outputImage;

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();

	NormalizationPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, NormalizationPassInput& input) override;
	void onResize() override;
};