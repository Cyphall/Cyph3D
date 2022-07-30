#pragma once

#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Scene/Scene.h"
#include <functional>

class Renderer
{
public:
	explicit Renderer(const char* name);
	virtual ~Renderer() = default;
	
	std::pair<Texture*, const PerfStep*> render(Camera& camera, bool debugView);
	
	virtual void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);
	
	virtual Entity* getClickedEntity(glm::ivec2 clickPos) = 0;
	
protected:
	std::unordered_map<std::string, Texture*> _textures;
	RenderRegistry _registry;
	
	const char* _name;
	
	PerfStep _renderPerf;
	GpuPerfCounter _perfCounter;
	
	virtual Texture& renderImpl(Camera& camera, Scene& scene, bool debugView) = 0;
	void render(RenderPass& pass, Camera& camera);
};