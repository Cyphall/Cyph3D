#pragma once

#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/Rendering/Pass/GeometryPass.h"
#include "Cyph3D/Rendering/Pass/GBufferDebugPass.h"
#include "Cyph3D/Rendering/Pass/SkyboxPass.h"
#include "Cyph3D/Rendering/Pass/LightingPass.h"
#include "Cyph3D/Rendering/Pass/ShadowMapPass.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/Rendering/Pass/ZPrePass.h"
#include "Cyph3D/Rendering/Renderer/Renderer.h"

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