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
	const std::shared_ptr<VKImage>& inputImage;
};

struct ToneMappingPassOutput
{
	const std::shared_ptr<VKImage>& outputImage;
};

class ToneMappingPass : public RenderPass<ToneMappingPassInput, ToneMappingPassOutput>
{
public:
	explicit ToneMappingPass(glm::uvec2 size);

private:
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

	ToneMappingPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, ToneMappingPassInput& input) override;
	void onResize() override;
};