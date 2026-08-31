#include "RasterizationSceneRenderer.h"

#include <Cyph3D/Asset/RuntimeAsset/CubemapAsset.h>

c3d::RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_exposurePass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
}

cgpu::ImagePtr c3d::RasterizationSceneRenderer::onRender(cgpu::CommandRecorder& commandRecorder, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	// Z prepass

	ZPrepassInput zPrepassInput{
		.registry = registry,
		.camera = camera
	};

	ZPrepassOutput zPrepassOutput = _zPrepass.render(commandRecorder, zPrepassInput);

	// Shadow map pass

	ShadowMapPassInput shadowMapPassInput{
		.registry = registry,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	ShadowMapPassOutput shadowMapPassOutput = _shadowMapPass.render(commandRecorder, shadowMapPassInput);

	// Lighting pass

	LightingPassInput lightingPassInput{
		.multisampledDepthImage = zPrepassOutput.multisampledDepthImage,
		.registry = registry,
		.camera = camera,
		.directionalShadowMapInfos = shadowMapPassOutput.directionalShadowMapInfos,
		.pointShadowMapInfos = shadowMapPassOutput.pointShadowMapInfos,
		.pointLightMaxDistance = shadowMapPassOutput.pointLightMaxDistance
	};

	LightingPassOutput lightingPassOutput = _lightingPass.render(commandRecorder, lightingPassInput);

	// Exposure pass

	ExposurePassInput exposurePassInput{
		.lightImage = lightingPassOutput.lightImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandRecorder, exposurePassInput);

	// Bloom pass

	BloomPassInput bloomPassInput{
		.lightImage = exposurePassOutput.lightImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandRecorder, bloomPassInput);

	// Tone mapping pass

	ToneMappingPassInput toneMappingPassInput{
		.lightImage = bloomPassOutput.lightImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandRecorder, toneMappingPassInput);

	return toneMappingPassOutput.colorImage;
}

void c3d::RasterizationSceneRenderer::onResize()
{
	_zPrepass.resize(_size);
	_shadowMapPass.resize(_size);
	_lightingPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}
