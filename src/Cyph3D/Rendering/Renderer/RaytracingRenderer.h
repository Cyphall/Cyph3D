#pragma once

#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/Rendering/Pass/RaytracePass.h"
#include "Cyph3D/Rendering/Renderer/Renderer.h"

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