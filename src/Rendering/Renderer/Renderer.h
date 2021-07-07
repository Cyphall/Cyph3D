#pragma once

#include "../RenderRegistry.h"
#include "../Pass/RenderPass.h"

class Renderer
{
public:
	virtual ~Renderer() = default;
	
	virtual Texture& render(Camera& camera, bool debugView) = 0;
	
	virtual void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);
	
	virtual Entity* getClickedEntity(glm::ivec2 clickPos) = 0;
	
protected:
	std::unordered_map<std::string, Texture*> _textures;
	RenderRegistry _registry;
	
	void render(RenderPass& pass, Camera& camera);
};
