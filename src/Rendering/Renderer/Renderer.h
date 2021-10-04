#pragma once

#include "../RenderRegistry.h"
#include "../Pass/RenderPass.h"
#include "../../Scene/Scene.h"

class Renderer
{
public:
	virtual ~Renderer() = default;
	
	Texture& render(Camera& camera, bool debugView);
	
	virtual void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);
	
	virtual Entity* getClickedEntity(glm::ivec2 clickPos) = 0;
	
protected:
	std::unordered_map<std::string, Texture*> _textures;
	RenderRegistry _registry;
	
	virtual Texture& renderImpl(Camera& camera, Scene& scene, bool debugView) = 0;
	void render(RenderPass& pass, Camera& camera);
};
