#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;

struct ExposurePassInput
{
	const std::shared_ptr<VKImage>& inputImage;
	const Camera& camera;
};

struct ExposurePassOutput
{
	const std::shared_ptr<VKImage>& outputImage;
};

class ExposurePass : public RenderPass<ExposurePassInput, ExposurePassOutput>
{
public:
	explicit ExposurePass(glm::uvec2 size);

private:
	struct PushConstantData
	{
		GLSL_float exposure;
	};

	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKSampler> _inputSampler;

	std::shared_ptr<VKImage> _outputImage;

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createSampler();
	void createImage();

	ExposurePassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, ExposurePassInput& input) override;
	void onResize() override;
};