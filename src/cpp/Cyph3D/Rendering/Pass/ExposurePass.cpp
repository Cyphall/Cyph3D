#include "ExposurePass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Scene/Camera.h>

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>

c3d::ExposurePass::ExposurePass(glm::uvec2 size):
	RenderPass(size, "Exposure pass")
{
	createPipelineState();
	createImage();
}

c3d::ExposurePassOutput c3d::ExposurePass::onRender(cgpu::CommandRecorder& commandRecorder, ExposurePassInput& input)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				Texture2D<>::Handle u_srcImage;
				WTexture2D<>::Handle u_dstImage;
				float u_exposure;
			} parameters{};

			parameters.u_size = _size;
			parameters.u_srcImage = ctx.getSampledImageDescriptor(input.lightImage);
			parameters.u_dstImage = ctx.getStorageImageDescriptor(_outputImage, cgpu::StorageAccess::eWriteonly);
			parameters.u_exposure = input.camera.getExposure();

			ctx.dispatch(_computeShaderState, {_size, 1}, {8, 8, 1}, parameters);
		},
	});

	return {
		.lightImage = _outputImage,
	};
}

void c3d::ExposurePass::onResize()
{
	createImage();
}

void c3d::ExposurePass::createPipelineState()
{
	_computeShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/exposure/exposure.slang"},
		}
	);
}

void c3d::ExposurePass::createImage()
{
	_outputImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Exposure output image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferSrc,
		}
	);
}
