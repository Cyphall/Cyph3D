#pragma once

#include "Renderer.h"
#include "../Pass/PostProcessingPass.h"
#include "../Pass/RaytracePass.h"

class RaytracingRenderer : public Renderer
{
public:
	RaytracingRenderer(glm::ivec2 size);
	
	Texture& render(Camera& camera, bool debugView) override;
	
	Entity* getClickedEntity(glm::ivec2 clickPos) override;
	
	static const char* identifier;

private:
	RaytracePass _raytracePass;
	PostProcessingPass _postProcessingPass;
	
	Framebuffer _objectIndexFramebuffer;
};
