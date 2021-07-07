#include "RaytracingRenderer.h"

const char* RaytracingRenderer::identifier = "Raytracing";

RaytracingRenderer::RaytracingRenderer(glm::ivec2 size):
_raytracePass(_textures, size),
_postProcessingPass(_textures, size)
{

}

Texture& RaytracingRenderer::render(Camera& camera, bool debugView)
{
	Renderer::render(_raytracePass, camera);
	Renderer::render(_postProcessingPass, camera);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return *_textures["final"];
}

Entity* RaytracingRenderer::getClickedEntity(glm::ivec2 clickPos)
{
	return nullptr;
}
