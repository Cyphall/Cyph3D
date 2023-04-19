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

class ExposureEffect : public PostProcessingEffect
{
public:
	explicit ExposureEffect(glm::uvec2 size);

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
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera) override;
	void onResize() override;
};