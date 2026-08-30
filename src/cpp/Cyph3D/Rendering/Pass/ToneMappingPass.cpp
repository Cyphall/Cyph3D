#include "ToneMappingPass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>

c3d::ToneMappingPass::ToneMappingPass(glm::uvec2 size):
	RenderPass(size, "Tone mapping pass")
{
	createPipelineState();
	createImage();
}

c3d::ToneMappingPassOutput c3d::ToneMappingPass::onRender(cgpu::CommandRecorder& commandRecorder, ToneMappingPassInput& input)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				Texture2D<>::Handle u_srcImage;
				WTexture2D<>::Handle u_dstImage;
			} parameters{};

			parameters.u_size = _size;
			parameters.u_srcImage = ctx.getSampledImageDescriptor(input.lightImage);
			parameters.u_dstImage = ctx.getStorageImageDescriptor(_outputImage, cgpu::StorageAccess::eWriteonly);

			ctx.dispatch(_computeShaderState, {cgpu::alignUp(_size, glm::uvec2{8u}) / 8u, 1}, parameters);
		},
	});

	return {
		.colorImage = _outputImage,
	};
}

void c3d::ToneMappingPass::onResize()
{
	createImage();
}

void c3d::ToneMappingPass::createPipelineState()
{
	_computeShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/tone mapping/tone mapping.slang"},
		}
	);
}

void c3d::ToneMappingPass::createImage()
{
	_outputImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Tone mapping output image",
			.format = SceneRenderer::FINAL_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferSrc,
		}
	);
}
