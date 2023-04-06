#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Scene/Scene.h"

#include <glad/glad.h>

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrePass(_textures, size),
	_shadowMapPass(_textures, size),
	_lightingPass(_textures, size),
	_skyboxPass(_textures, size),
	_postProcessingPass(_textures, size)
{
	_objectIndexFramebuffer.attachColor(0, *_textures["object_index"]);
	_objectIndexFramebuffer.setReadBuffer(0);
}

GLTexture& RasterizationSceneRenderer::renderImpl(Camera& camera, Scene& scene)
{
	SceneRenderer::render(_zPrePass, camera);
	SceneRenderer::render(_shadowMapPass, camera);

	SceneRenderer::render(_lightingPass, camera);
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isLoaded())
		SceneRenderer::render(_skyboxPass, camera);
	
	SceneRenderer::render(_postProcessingPass, camera);
	
	return *_textures["final"];
}

void RasterizationSceneRenderer::onNewFrame()
{
	SceneRenderer::onNewFrame();
}

Entity* RasterizationSceneRenderer::getClickedEntity(glm::uvec2 clickPos)
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