#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/Rendering/Pass/PostProcessingPass.h"
#include "Cyph3D/Rendering/Pass/RaytracePass.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"

class RaytracingSceneRenderer : public SceneRenderer
{
public:
	explicit RaytracingSceneRenderer(glm::ivec2 size);
	
	Entity* getClickedEntity(glm::ivec2 clickPos) override;
	
	static const char* identifier;

private:
	RaytracePass _raytracePass;
	PostProcessingPass _postProcessingPass;
	
	GLFramebuffer _objectIndexFramebuffer;
	
	GLTexture& renderImpl(Camera& camera, Scene& scene) override;
};