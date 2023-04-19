#pragma once

#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;
class VKImageView;

class ToneMappingEffect : public PostProcessingEffect
{
public:
	explicit ToneMappingEffect(glm::uvec2 size);
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera) override;

private:
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKSampler> _inputSampler;
	
	VKDynamic<VKImage> _outputImage;
	VKDynamic<VKImageView> _outputLinearImageView;
	VKDynamic<VKImageView> _outputSrgbImageView;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createSampler();
	void createImage();
};