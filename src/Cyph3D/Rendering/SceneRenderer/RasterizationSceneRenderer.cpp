#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Scene/Scene.h"

#include <glad/glad.h>

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_skyboxPass(size),
	_postProcessingPass(size)
{
	_objectIndexFramebuffer.setReadBuffer(0);
}

GLTexture& RasterizationSceneRenderer::renderImpl(Camera& camera)
{
	ZPrepassInput zPrepassInput{
		.registry = _registry,
		.camera = camera
	};
	
	ZPrepassOutput zPrepassOutput = _zPrepass.render(zPrepassInput, _renderPerf);
	
	ShadowMapPassInput shadowMapPassInput{
		.registry = _registry,
		.camera = camera
	};
	
	_shadowMapPass.render(shadowMapPassInput, _renderPerf);
	
	LightingPassInput lightingPassInput{
		.depth = zPrepassOutput.depth,
		.registry = _registry,
		.camera = camera
	};
	
	LightingPassOutput lightingPassOutput = _lightingPass.render(lightingPassInput, _renderPerf);
	_objectIndexFramebuffer.attachColor(0, lightingPassOutput.objectIndex);
	
	Scene& scene = Engine::getScene();
	
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isLoaded())
	{
		SkyboxPassInput skyboxPassInput{
			.camera = camera,
			.rawRender = lightingPassOutput.rawRender,
			.depth = zPrepassOutput.depth
		};
		
		_skyboxPass.render(skyboxPassInput, _renderPerf);
	}
	
	PostProcessingPassInput postProcessingPassInput{
		.rawRender = lightingPassOutput.rawRender,
		.camera = camera
	};
	
	PostProcessingPassOutput postProcessingPassOutput = _postProcessingPass.render(postProcessingPassInput, _renderPerf);
	
	return postProcessingPassOutput.postProcessedRender;
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