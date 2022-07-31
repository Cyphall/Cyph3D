#include "RaytracingRenderer.h"

#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"

const char* RaytracingRenderer::identifier = "Raytracing";

RaytracingRenderer::RaytracingRenderer(glm::ivec2 size):
Renderer("Raytracing Renderer"),
_raytracePass(_textures, size),
_postProcessingPass(_textures, size),
_objectIndexFramebuffer(size)
{
	_objectIndexFramebuffer.attachColor(0, *_textures["objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

Texture& RaytracingRenderer::renderImpl(Camera& camera, Scene& scene, bool debugView)
{
	Renderer::render(_raytracePass, camera);
	Renderer::render(_postProcessingPass, camera);
	
	return *_textures["final"];
}

Entity* RaytracingRenderer::getClickedEntity(glm::ivec2 clickPos)
{
	int objectIndex;
	_objectIndexFramebuffer.bindForReading();
	// shift origin from bottom left to top left
	clickPos.y = _objectIndexFramebuffer.getSize().y - clickPos.y;
	
	glReadPixels(clickPos.x, clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
	
	if (objectIndex != -1)
	{
		return _registry.shapes[objectIndex].owner;
	}
	
	return nullptr;
}