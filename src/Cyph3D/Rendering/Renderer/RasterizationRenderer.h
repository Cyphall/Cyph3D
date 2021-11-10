#pragma once

#include "../../GLObject/Framebuffer.h"
#include "../../GLObject/VertexArray.h"
#include "../../GLObject/ShaderStorageBuffer.h"
#include "../PostProcessingEffect/PostProcessingEffect.h"
#include "../Pass/GeometryPass.h"
#include "../Pass/GBufferDebugPass.h"
#include "../Pass/SkyboxPass.h"
#include "../Pass/LightingPass.h"
#include "../Pass/ShadowMapPass.h"
#include "../Pass/PostProcessingPass.h"
#include "../Pass/ZPrePass.h"
#include "Renderer.h"

class RasterizationRenderer : public Renderer
{
public:
	explicit RasterizationRenderer(glm::ivec2 size);
	
	void onNewFrame() override;
	
	Entity* getClickedEntity(glm::ivec2 clickPos) override;
	
	static const char* identifier;

private:
	ZPrePass _zPrePass;
	ShadowMapPass _shadowMapPass;
	GeometryPass _geometryPass;
	GBufferDebugPass _gBufferDebugPass;
	SkyboxPass _skyboxPass;
	LightingPass _lightingPass;
	PostProcessingPass _postProcessingPass;
	
	Framebuffer _objectIndexFramebuffer;
	
	Texture& renderImpl(Camera& camera, Scene& scene, bool debugView) override;
};
