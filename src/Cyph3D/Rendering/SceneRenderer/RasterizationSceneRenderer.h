#pragma once

#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/Pass/ZPrepass.h"
#include "Cyph3D/Rendering/Pass/ShadowMapPass.h"
#include "Cyph3D/Rendering/Pass/LightingPass.h"
#include "Cyph3D/Rendering/Pass/SkyboxPass.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/GLObject/GLFramebuffer.h"

class RasterizationSceneRenderer : public SceneRenderer
{
public:
	explicit RasterizationSceneRenderer(glm::uvec2 size);
	
	void onNewFrame() override;
	
	Entity* getClickedEntity(glm::uvec2 clickPos) override;

private:
	ZPrepass _zPrepass;
	ShadowMapPass _shadowMapPass;
	LightingPass _lightingPass;
	SkyboxPass _skyboxPass;
	PostProcessingPass _postProcessingPass;
	
	GLFramebuffer _objectIndexFramebuffer;
	
	GLTexture& renderImpl(Camera& camera) override;
};