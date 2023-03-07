#pragma once

#include "Cyph3D/PerfCounter/GpuPerfCounter.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Rendering/RenderRegistry.h"

#include <glm/glm.hpp>

class RenderPass;
class Scene;
class GLTexture;
class Camera;

class Renderer
{
public:
	Renderer(const char* name, glm::ivec2 size);
	virtual ~Renderer() = default;
	
	std::pair<GLTexture*, const PerfStep*> render(Camera& camera, bool debugView);
	
	virtual void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);

	glm::ivec2 getSize() const;
	
	virtual Entity* getClickedEntity(glm::ivec2 clickPos) = 0;
	
protected:
	std::unordered_map<std::string, GLTexture*> _textures;
	RenderRegistry _registry;
	
	const char* _name;
	glm::ivec2 _size;

	PerfStep _renderPerf;
	GpuPerfCounter _perfCounter;
	
	virtual GLTexture& renderImpl(Camera& camera, Scene& scene, bool debugView) = 0;
	void render(RenderPass& pass, Camera& camera);
};