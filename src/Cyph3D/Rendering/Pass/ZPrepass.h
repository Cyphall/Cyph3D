#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
class VKImageView;

struct ZPrepassInput
{
	RenderRegistry& registry;
	Camera& camera;
};

struct ZPrepassOutput
{
	const VKPtr<VKImageView>& depthView;
};

class ZPrepass : public RenderPass<ZPrepassInput, ZPrepassOutput>
{
public:
	explicit ZPrepass(glm::uvec2 size);
	
private:
	struct PushConstantData
	{
		GLSL_mat4 mvp;
	};
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKDynamic<VKImage> _depthMap;
	VKDynamic<VKImageView> _depthMapView;
	
	ZPrepassOutput renderImpl(const VKPtr<VKCommandBuffer>& commandBuffer, ZPrepassInput& input) override;
	
	void createPipelineLayout();
	void createPipeline();
	void createDepthMap();
};