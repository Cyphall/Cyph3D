#pragma once

#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/Pass/ZPrepass.h"
#include "Cyph3D/Rendering/Pass/ShadowMapPass.h"
#include "Cyph3D/Rendering/Pass/LightingPass.h"
#include "Cyph3D/Rendering/Pass/SkyboxPass.h"
#include "Cyph3D/Rendering/Pass/ExposurePass.h"
#include "Cyph3D/Rendering/Pass/BloomPass.h"
#include "Cyph3D/Rendering/Pass/ToneMappingPass.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

class RasterizationSceneRenderer : public SceneRenderer
{
public:
	explicit RasterizationSceneRenderer(glm::uvec2 size);

private:
	ZPrepass _zPrepass;
	ShadowMapPass _shadowMapPass;
	LightingPass _lightingPass;
	SkyboxPass _skyboxPass;
	ExposurePass _exposurePass;
	BloomPass _bloomPass;
	ToneMappingPass _toneMappingPass;
	
	const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry) override;
	void onResize() override;
};