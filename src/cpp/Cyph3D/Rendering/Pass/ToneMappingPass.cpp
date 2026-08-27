#include "ToneMappingPass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/Sampler.hpp>
#include <CyphGPU/VertexInputState.hpp>

c3d::ToneMappingPass::ToneMappingPass(glm::uvec2 size):
	RenderPass(size, "Tone mapping pass")
{
	createPipelineStates();
	createImage();
}

c3d::ToneMappingPassOutput c3d::ToneMappingPass::onRender(cgpu::CommandRecorder& commandRecorder, ToneMappingPassInput& input)
{
	commandRecorder.graphicsPass({
		.color_attachments = {{
			{
				.image = _outputImage,
				.load_op = vk::AttachmentLoadOp::eDontCare,
				.store_op = vk::AttachmentStoreOp::eStore,
			},
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_preRasterizationShaderState,
				_fragmentShaderState,
				_fragmentOutputState
			);

			using namespace cgpu::shader_types;
			struct
			{
				Texture2D<>::Handle u_image;
			} parameters{};

			parameters.u_image = ctx.getSampledImageDescriptor(
				input.lightImage,
				cgpu::GraphicsStage::eFragment
			);

			ctx.draw(3, 1, 0, 0, parameters);
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

void c3d::ToneMappingPass::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_preRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/fullscreen quad.slang"},
		}
	);

	_fragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/post-processing/tone mapping/tone mapping.slang"}},
		}
	);

	_fragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.color_attachments = {
				{
					.format = SceneRenderer::FINAL_COLOR_FORMAT,
				},
			},
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
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferSrc,
		}
	);
}
