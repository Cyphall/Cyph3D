#include "NormalizationPass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>

c3d::NormalizationPass::NormalizationPass(glm::uvec2 size):
	RenderPass(size, "Normalization pass")
{
	createPipelineState();
	createImage();
}

c3d::NormalizationPassOutput c3d::NormalizationPass::onRender(cgpu::CommandRecorder& commandRecorder, NormalizationPassInput& input)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				Texture2D<>::Handle u_srcImage;
				WTexture2D<>::Handle u_dstImage;
				uint32_t u_accumulatedSamples;
				uint2 u_size;
			} parameters{};

			parameters.u_srcImage = ctx.getSampledImageDescriptor(input.lightImage);
			parameters.u_dstImage = ctx.getStorageImageDescriptor(_outputImage, cgpu::StorageAccess::eWriteonly);
			parameters.u_accumulatedSamples = input.accumulatedSamples;
			parameters.u_size = _size;

			ctx.dispatch(_computeShaderState, {_size, 1}, {8, 8, 1}, parameters);
		},
	});

	return {
		.lightImage = _outputImage,
	};
}

void c3d::NormalizationPass::onResize()
{
	createImage();
}

void c3d::NormalizationPass::createPipelineState()
{
	_computeShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/normalization/normalization.slang"},
		}
	);
}

void c3d::NormalizationPass::createImage()
{
	_outputImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Normalization output image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferSrc,
		}
	);
}
