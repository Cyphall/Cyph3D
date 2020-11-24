#pragma once

#include "../GLObject/Framebuffer.h"
#include "../GLObject/VertexArray.h"
#include "../GLObject/VertexBuffer.h"
#include "../GLObject/ShaderStorageBuffer.h"
#include "../Scene/PointLight.h"
#include "../Scene/DirectionalLight.h"
#include "../Scene/MeshObject.h"
#include "IPostProcessEffect.h"
#include "Pass/GeometryPass.h"
#include "Pass/SkyboxPass.h"
#include "Pass/LightingPass.h"
#include "Pass/ShadowMapPass.h"
#include "Pass/PostProcessingPass.h"

class Renderer
{
public:
	Renderer();
	
	bool getDebug() const;
	void setDebug(bool debug);
	
	void render();

private:
	// GBuffer
	bool _debug = false;
	
	std::unordered_map<std::string, Texture*> _textures;
	
	ShadowMapPass _shadowMapPass;
	GeometryPass _geometryPass;
	SkyboxPass _skyboxPass;
	LightingPass _lightingPass;
	PostProcessingPass _postProcessingPass;
	
	void render(IRenderPass& pass, SceneObjectRegistry& registry, Camera& camera);
};
