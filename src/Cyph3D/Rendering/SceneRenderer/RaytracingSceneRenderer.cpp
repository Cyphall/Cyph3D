#include "RaytracingSceneRenderer.h"

RaytracingSceneRenderer::RaytracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Raytracing SceneRenderer", size),
	_raytracePass(size),
	_postProcessingPass(size)
{
	_objectIndexFramebuffer.setReadBuffer(0);
}

GLTexture& RaytracingSceneRenderer::renderImpl(Camera& camera)
{
	RaytracePassInput raytracePassInput{
		.registry = _registry,
		.camera = camera
	};
	
	RaytracePassOutput raytracePassOutput = _raytracePass.render(raytracePassInput, _renderPerf);
	_objectIndexFramebuffer.attachColor(0, raytracePassOutput.objectIndex);
	
	PostProcessingPassInput postProcessingPassInput{
		.rawRender = raytracePassOutput.rawRender,
		.camera = camera
	};
	
	PostProcessingPassOutput postProcessingPassOutput = _postProcessingPass.render(postProcessingPassInput, _renderPerf);
	
	return postProcessingPassOutput.postProcessedRender;
}

Entity* RaytracingSceneRenderer::getClickedEntity(glm::uvec2 clickPos)
{
	int objectIndex;
	_objectIndexFramebuffer.bindForReading();
	// shift origin from bottom left to top left
	clickPos.y = _size.y - clickPos.y;
	
	glReadPixels(clickPos.x, clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
	
	if (objectIndex != -1)
	{
		return _registry.shapes[objectIndex].owner;
	}
	
	return nullptr;
}