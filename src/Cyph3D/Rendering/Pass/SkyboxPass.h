#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
class VKImageView;
template<typename T>
class VKBuffer;
class VKSampler;

struct SkyboxPassInput
{
	Camera& camera;
	const VKPtr<VKImageView>& rawRenderView;
	const VKPtr<VKImageView>& depthView;
};

struct SkyboxPassOutput
{

};

class SkyboxPass : public RenderPass<SkyboxPassInput, SkyboxPassOutput>
{
public:
	explicit SkyboxPass(glm::uvec2 size);

private:
	struct VertexData
	{
		glm::vec3 position;
	};
	
	struct PushConstantData
	{
		GLSL_mat4 mvp;
	};
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKBuffer<SkyboxPass::VertexData>> _vertexBuffer;
	
	VKPtr<VKSampler> _sampler;
	
	SkyboxPassOutput renderImpl(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input) override;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createBuffer();
	void createSampler();
};