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

struct ExposurePassInput
{
	const VKPtr<VKImageView>& inputImageView;
	const Camera& camera;
};

struct ExposurePassOutput
{
	const VKPtr<VKImageView>& outputImageView;
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
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKSampler> _inputSampler;
	
	VKDynamic<VKImage> _outputImage;
	VKDynamic<VKImageView> _outputImageView;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createSampler();
	void createImage();
	
	ExposurePassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ExposurePassInput& input) override;
	void onResize() override;
};