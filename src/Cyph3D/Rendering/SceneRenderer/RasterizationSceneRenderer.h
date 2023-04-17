#pragma once

#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/Pass/ZPrepass.h"
#include "Cyph3D/Rendering/Pass/ShadowMapPass.h"
#include "Cyph3D/Rendering/Pass/LightingPass.h"
#include "Cyph3D/Rendering/Pass/SkyboxPass.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

class RasterizationSceneRenderer : public SceneRenderer
{
public:
	explicit RasterizationSceneRenderer(glm::uvec2 size);
	
	Entity* getClickedEntity(glm::uvec2 clickPos) override;

private:
	ZPrepass _zPrepass;
	ShadowMapPass _shadowMapPass;
	LightingPass _lightingPass;
	SkyboxPass _skyboxPass;
	PostProcessingPass _postProcessingPass;
	
	VKPtr<VKBuffer<int32_t>> _objectIndexBuffer;
	VKPtr<VKImageView> _objectIndexView;
	
	const VKPtr<VKImageView>& renderImpl(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera) override;
};