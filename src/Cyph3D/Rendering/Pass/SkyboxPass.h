#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
template<typename T>
class VKBuffer;

struct SkyboxPassInput
{
	Camera& camera;
	const std::shared_ptr<VKImage>& multisampledRawRenderImage;
	const std::shared_ptr<VKImage>& multisampledDepthImage;
};

struct SkyboxPassOutput
{
	const std::shared_ptr<VKImage>& rawRenderImage;
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

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKImage> _resolvedRawRenderImage;

	std::shared_ptr<VKBuffer<SkyboxPass::VertexData>> _vertexBuffer;

	SkyboxPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input) override;
	void onResize() override;

	void createPipelineLayout();
	void createPipeline();
	void createImages();
	void createBuffer();
};