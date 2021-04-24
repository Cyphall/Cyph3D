#pragma once

#include "../GLObject/Framebuffer.h"
#include "../GLObject/VertexArray.h"
#include "../GLObject/ShaderStorageBuffer.h"
#include "../Scene/PointLight.h"
#include "../Scene/DirectionalLight.h"
#include "../Scene/MeshObject.h"
#include "IPostProcessingEffect.h"
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
	Renderer();
	
	bool getDebug() const;
	void setDebug(bool debug);
	
	void render();
	
	MeshObject* getClickedMeshObject(glm::dvec2 clickPos);
	
	std::unordered_map<std::string, Texture*>& getTextures();
	SceneObjectRegistry& getRegistry();

private:
	// GBuffer
	bool _debug = false;
	
	std::unordered_map<std::string, Texture*> _textures;
	SceneObjectRegistry _registry;
	
	ZPrePass _zPrePass;
	ShadowMapPass _shadowMapPass;
	GeometryPass _geometryPass;
	GBufferDebugPass _gBufferDebugPass;
	SkyboxPass _skyboxPass;
	LightingPass _lightingPass;
	PostProcessingPass _postProcessingPass;
	
	Framebuffer _objectIndexFramebuffer;
	
	void render(IRenderPass& pass, Camera& camera);
};
