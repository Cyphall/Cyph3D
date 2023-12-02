#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKSampler;
class VKImage;

struct ToneMappingPassInput
{
	const VKPtr<VKImage>& inputImage;
};

struct ToneMappingPassOutput
{
	const VKPtr<VKImage>& outputImage;
};

class ToneMappingPass : public RenderPass<ToneMappingPassInput, ToneMappingPassOutput>
{
public:
	explicit ToneMappingPass(glm::uvec2 size);

private:
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;

	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;

	VKPtr<VKSampler> _inputSampler;

	VKPtr<VKImage> _outputImage;

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createSampler();
	void createImage();

	ToneMappingPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ToneMappingPassInput& input) override;
	void onResize() override;
};