#include <glad/glad.h>
#include "RasterizationRenderer.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"

const char* RasterizationRenderer::identifier = "Rasterisation";

RasterizationRenderer::RasterizationRenderer(glm::ivec2 size):
_zPrePass(_textures, size),
_shadowMapPass(_textures, size),
_geometryPass(_textures, size),
_gBufferDebugPass(_textures, size),
_skyboxPass(_textures, size),
_lightingPass(_textures, size),
_postProcessingPass(_textures, size),
_objectIndexFramebuffer(size)
{
	_objectIndexFramebuffer.attachColor(0, *_textures["gbuffer_objectIndex"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

Texture& RasterizationRenderer::renderImpl(Camera& camera, Scene& scene, bool debugView)
{
	Renderer::render(_zPrePass, camera);
	Renderer::render(_shadowMapPass, camera);
	
	Renderer::render(_geometryPass, camera);
	
	if (debugView)
	{
		Renderer::render(_gBufferDebugPass, camera);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *_textures["gbuffer_debug"];
	}
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isResourceReady())
		Renderer::render(_skyboxPass, camera);
	
	Renderer::render(_lightingPass, camera);
	
	Renderer::render(_postProcessingPass, camera);
	
	return *_textures["final"];
}

void RasterizationRenderer::onNewFrame()
{
	Renderer::onNewFrame();
}

Entity* RasterizationRenderer::getClickedEntity(glm::ivec2 clickPos)
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
