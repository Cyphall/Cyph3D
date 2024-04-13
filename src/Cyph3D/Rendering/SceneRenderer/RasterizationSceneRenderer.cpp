#include "RasterizationSceneRenderer.h"

#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

RasterizationSceneRenderer::RasterizationSceneRenderer(glm::uvec2 size):
	SceneRenderer("Rasterization SceneRenderer", size),
	_zPrepass(size),
	_shadowMapPass(size),
	_lightingPass(size),
	_skyboxPass(size),
	_exposurePass(size),
	_bloomPass(size),
	_toneMappingPass(size)
{
}

const VKPtr<VKImage>& RasterizationSceneRenderer::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, Camera& camera, const RenderRegistry& registry, bool sceneChanged, bool cameraChanged)
{
	// Z prepass

	ZPrepassInput zPrepassInput{
		.registry = registry,
		.camera = camera
	};

	ZPrepassOutput zPrepassOutput = _zPrepass.render(commandBuffer, zPrepassInput);

	// Shadow map pass

	ShadowMapPassInput shadowMapPassInput{
		.registry = registry,
		.sceneChanged = sceneChanged,
		.cameraChanged = cameraChanged
	};

	ShadowMapPassOutput shadowMapPassOutput = _shadowMapPass.render(commandBuffer, shadowMapPassInput);

	// Lighting pass

	LightingPassInput lightingPassInput{
		.multisampledDepthImage = zPrepassOutput.multisampledDepthImage,
		.registry = registry,
		.camera = camera,
		.directionalShadowMapInfos = shadowMapPassOutput.directionalShadowMapInfos,
		.pointShadowMapInfos = shadowMapPassOutput.pointShadowMapInfos
	};

	LightingPassOutput lightingPassOutput = _lightingPass.render(commandBuffer, lightingPassInput);

	// Skybox pass

	SkyboxPassInput skyboxPassInput{
		.camera = camera,
		.multisampledRawRenderImage = lightingPassOutput.multisampledRawRenderImage,
		.multisampledDepthImage = zPrepassOutput.multisampledDepthImage
	};

	SkyboxPassOutput skyboxPassOutput = _skyboxPass.render(commandBuffer, skyboxPassInput);

	// Exposure pass

	ExposurePassInput exposurePassInput{
		.inputImage = skyboxPassOutput.rawRenderImage,
		.camera = camera
	};

	ExposurePassOutput exposurePassOutput = _exposurePass.render(commandBuffer, exposurePassInput);

	// Bloom pass

	BloomPassInput bloomPassInput{
		.inputImage = exposurePassOutput.outputImage
	};

	BloomPassOutput bloomPassOutput = _bloomPass.render(commandBuffer, bloomPassInput);

	// Tone mapping pass

	ToneMappingPassInput toneMappingPassInput{
		.inputImage = bloomPassOutput.outputImage
	};

	ToneMappingPassOutput toneMappingPassOutput = _toneMappingPass.render(commandBuffer, toneMappingPassInput);

	return toneMappingPassOutput.outputImage;
}

void RasterizationSceneRenderer::onResize()
{
	_zPrepass.resize(_size);
	_shadowMapPass.resize(_size);
	_lightingPass.resize(_size);
	_skyboxPass.resize(_size);
	_exposurePass.resize(_size);
	_bloomPass.resize(_size);
	_toneMappingPass.resize(_size);
}