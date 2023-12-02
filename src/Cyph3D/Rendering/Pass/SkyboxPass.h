#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKPtr.h"

class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
template<typename T>
class VKBuffer;

struct SkyboxPassInput
{
	Camera& camera;
	const VKPtr<VKImage>& multisampledRawRenderImage;
	const VKPtr<VKImage>& multisampledDepthImage;
};

struct SkyboxPassOutput
{
	const VKPtr<VKImage>& rawRenderImage;
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

	VKPtr<VKImage> _resolvedRawRenderImage;

	VKPtr<VKBuffer<SkyboxPass::VertexData>> _vertexBuffer;

	SkyboxPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input) override;
	void onResize() override;

	void createPipelineLayout();
	void createPipeline();
	void createImages();
	void createBuffer();
};