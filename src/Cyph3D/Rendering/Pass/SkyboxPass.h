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
	const VKPtr<VKImageView>& multisampledRawRenderImageView;
	const VKPtr<VKImageView>& multisampledDepthImageView;
};

struct SkyboxPassOutput
{
	const VKPtr<VKImageView>& rawRenderImageView;
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
		GLSL_uint textureIndex;
	};
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKDynamic<VKImage> _resolvedRawRenderImage;
	VKDynamic<VKImageView> _resolvedRawRenderImageView;
	
	VKPtr<VKBuffer<SkyboxPass::VertexData>> _vertexBuffer;
	
	VKPtr<VKSampler> _sampler;
	
	SkyboxPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input) override;
	void onResize() override;
	
	void createPipelineLayout();
	void createPipeline();
	void createImages();
	void createBuffer();
	void createSampler();
};