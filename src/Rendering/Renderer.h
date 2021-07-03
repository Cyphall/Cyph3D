#pragma once

#include "../GLObject/Framebuffer.h"
#include "../GLObject/VertexArray.h"
#include "../GLObject/ShaderStorageBuffer.h"
#include "PostProcessingEffect/PostProcessingEffect.h"
#include "Pass/GeometryPass.h"
#include "Pass/GBufferDebugPass.h"
#include "Pass/SkyboxPass.h"
#include "Pass/LightingPass.h"
#include "Pass/ShadowMapPass.h"
#include "Pass/PostProcessingPass.h"
#include "Pass/ZPrePass.h"

class Renderer
{
public:
	Renderer(glm::ivec2 size);
	
	Texture& render(Camera& camera, bool debugView);
	
	void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);
	
	Entity* getClickedEntity(glm::ivec2 clickPos);
	
	std::unordered_map<std::string, Texture*>& getTextures();
	RenderRegistry& getRegistry();

private:
	std::unordered_map<std::string, Texture*> _textures;
	RenderRegistry _registry;
	
	ZPrePass _zPrePass;
	ShadowMapPass _shadowMapPass;
	GeometryPass _geometryPass;
	GBufferDebugPass _gBufferDebugPass;
	SkyboxPass _skyboxPass;
	LightingPass _lightingPass;
	PostProcessingPass _postProcessingPass;
	
	Framebuffer _objectIndexFramebuffer;
	
	void render(RenderPass& pass, Camera& camera);
};
