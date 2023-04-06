#include "RaytracingSceneRenderer.h"

RaytracingSceneRenderer::RaytracingSceneRenderer(glm::uvec2 size):
	SceneRenderer("Raytracing SceneRenderer", size),
	_raytracePass(_textures, size),
	_postProcessingPass(_textures, size)
{
	_objectIndexFramebuffer.attachColor(0, *_textures["objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

GLTexture& RaytracingSceneRenderer::renderImpl(Camera& camera, Scene& scene)
{
	SceneRenderer::render(_raytracePass, camera);
	SceneRenderer::render(_postProcessingPass, camera);
	
	return *_textures["final"];
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