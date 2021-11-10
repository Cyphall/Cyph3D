#pragma once

#include "Renderer.h"
#include "../Pass/PostProcessingPass.h"
#include "../Pass/RaytracePass.h"

class RaytracingRenderer : public Renderer
{
public:
	explicit RaytracingRenderer(glm::ivec2 size);
	
	Entity* getClickedEntity(glm::ivec2 clickPos) override;
	
	static const char* identifier;

private:
	RaytracePass _raytracePass;
	PostProcessingPass _postProcessingPass;
	
	Framebuffer _objectIndexFramebuffer;
	
	Texture& renderImpl(Camera& camera, Scene& scene, bool debugView) override;
};
