#pragma once

#include "Cyph3D/PerfCounter/GpuPerfCounter.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Rendering/RenderRegistry.h"

#include <glm/glm.hpp>
#include <string>
#include <string_view>

class RenderPass;
class Scene;
class GLTexture;
class Camera;

class SceneRenderer
{
public:
	SceneRenderer(std::string_view name, glm::uvec2 size);
	virtual ~SceneRenderer() = default;
	
	GLTexture& render(Camera& camera);
	
	virtual void onNewFrame();
	
	void requestShapeRendering(ShapeRenderer::RenderData request);
	void requestLightRendering(DirectionalLight::RenderData data);
	void requestLightRendering(PointLight::RenderData data);

	glm::uvec2 getSize() const;
	
	const PerfStep& getRenderPerf();
	
	virtual Entity* getClickedEntity(glm::uvec2 clickPos) = 0;
	
protected:
	std::unordered_map<std::string, GLTexture*> _textures;
	RenderRegistry _registry;
	
	std::string _name;
	glm::uvec2 _size;

	PerfStep _renderPerf;
	GpuPerfCounter _perfCounter;
	
	virtual GLTexture& renderImpl(Camera& camera, Scene& scene) = 0;
	void render(RenderPass& pass, Camera& camera);
};