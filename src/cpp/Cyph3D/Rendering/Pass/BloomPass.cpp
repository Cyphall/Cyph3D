#include "BloomPass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/Sampler.hpp>
#include <CyphGPU/VertexInputState.hpp>

namespace
{
constexpr float BLOOM_RADIUS = 0.85f;
constexpr float BLOOM_STRENGTH = 0.15f;
}

c3d::BloomPass::BloomPass(glm::uvec2 size):
	RenderPass(size, "Bloom pass")
{
	createPipelineStates();
	createImages();
	createSamplers();
}

c3d::BloomPassOutput c3d::BloomPass::onRender(cgpu::CommandRecorder& commandRecorder, BloomPassInput& input)
{
	// copy inputImageView image level 0 to work image level 0
	{
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, "copyImageBaseLevel"};
		commandRecorder.copyImageToImage({
			.src_image = input.lightImage,
			.dst_image = _workImage,
			.ranges = {{
				{
					.dst = {{
						.level = 0,
					}},
				},
			}},
		});
	}

	// downsample work image
	for (size_t i = 0; i < _workImage->getDesc().levels - 1; i++)
	{
		uint32_t srcLevel = i + 0;
		uint32_t dstLevel = i + 1;
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("downsampleAndBlur({}->{})", srcLevel, dstLevel)};
		downsampleAndBlur(commandRecorder, srcLevel, dstLevel);
	}

	// upsample and blur work image
	for (size_t i = 0; i < _workImage->getDesc().levels - 1; i++)
	{
		uint32_t srcLevel = _workImage->getDesc().levels - 1 - i;
		uint32_t dstLevel = _workImage->getDesc().levels - 2 - i;
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("upsampleAndBlur({}->{})", srcLevel, dstLevel)};
		upsampleAndBlur(commandRecorder, srcLevel, dstLevel);
	}

	// compose inputImageView image level 0 and work image level 0 to outputImageView image level 0
	{
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, "compose"};
		compose(commandRecorder, input.lightImage);
	}

	return {
		.lightImage = _outputImage,
	};
}

void c3d::BloomPass::onResize()
{
	createImages();
}

void c3d::BloomPass::downsampleAndBlur(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel)
{
	commandRecorder.graphicsPass({
		.color_attachments = {{
			{
				.image = _workImage,
				.level = dstLevel,
				.load_op = vk::AttachmentLoadOp::eDontCare,
				.store_op = vk::AttachmentStoreOp::eStore,
			},
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_preRasterizationShaderState,
				_downsampleFragmentShaderState,
				_downsampleComposeFragmentOutputState
			);

			using namespace cgpu::shader_types;
			struct
			{
				Texture2D<>::Handle u_image;
				SamplerState::Handle u_sampler;
				float2 u_srcPixelSize;
			} parameters{};

			parameters.u_image = ctx.getSampledImageDescriptor(
				_workImage,
				cgpu::GraphicsStage::eFragment,
				{.levels = {{srcLevel, 1}}}
			);
			parameters.u_sampler = _downsampleSampler->getDescriptor();
			parameters.u_srcPixelSize = glm::vec2{1.0f} / glm::vec2{_workImage->calcLevelExtent(srcLevel)};

			ctx.draw(3, 1, 0, 0, parameters);
		},
	});
}

void c3d::BloomPass::upsampleAndBlur(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel)
{
	commandRecorder.graphicsPass({
		.color_attachments = {{
			{
				.image = _workImage,
				.level = dstLevel,
				.load_op = vk::AttachmentLoadOp::eLoad,
				.store_op = vk::AttachmentStoreOp::eStore,
			},
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_preRasterizationShaderState,
				_upsampleFragmentShaderState,
				_upsampleFragmentOutputState
			);

			using namespace cgpu::shader_types;
			struct
			{
				Texture2D<>::Handle u_image;
				SamplerState::Handle u_sampler;
				float2 u_srcPixelSize;
				float u_bloomRadius;
			} parameters{};

			parameters.u_image = ctx.getSampledImageDescriptor(
				_workImage,
				cgpu::GraphicsStage::eFragment,
				{.levels = {{srcLevel, 1}}}
			);
			parameters.u_sampler = _upsampleSampler->getDescriptor();
			parameters.u_srcPixelSize = glm::vec2{1.0f} / glm::vec2{_workImage->calcLevelExtent(srcLevel)};
			parameters.u_bloomRadius = glm::clamp(BLOOM_RADIUS, 0.0f, 1.0f);

			ctx.draw(3, 1, 0, 0, parameters);
		},
	});
}

void c3d::BloomPass::compose(cgpu::CommandRecorder& commandRecorder, const cgpu::ImagePtr& input)
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
				_composeFragmentShaderState,
				_downsampleComposeFragmentOutputState
			);

			using namespace cgpu::shader_types;
			struct
			{
				Texture2D<>::Handle u_srcAImage;
				Texture2D<>::Handle u_srcBImage;
				SamplerState::Handle u_sampler;
				float u_factor;
			} parameters{};

			parameters.u_srcAImage = ctx.getSampledImageDescriptor(
				input,
				cgpu::GraphicsStage::eFragment
			);
			parameters.u_srcBImage = ctx.getSampledImageDescriptor(
				_workImage,
				cgpu::GraphicsStage::eFragment,
				{.levels = {{0, 1}}}
			);
			parameters.u_sampler = _composeSampler->getDescriptor();
			parameters.u_factor = glm::clamp(BLOOM_STRENGTH, 0.0f, 1.0f);

			ctx.draw(3, 1, 0, 0, parameters);
		},
	});
}

void c3d::BloomPass::createPipelineStates()
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

	_downsampleFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/post-processing/bloom/downsample.slang"}},
		}
	);

	_upsampleFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/post-processing/bloom/upsample.slang"}},
		}
	);

	_composeFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/post-processing/bloom/compose.slang"}},
		}
	);

	_downsampleComposeFragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.color_attachments = {
				{
					.format = SceneRenderer::HDR_COLOR_FORMAT,
				},
			},
		}
	);

	_upsampleFragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.color_attachments = {
				{
					.format = SceneRenderer::HDR_COLOR_FORMAT,
					.blend = {{
						.color = {
							.src_factor = vk::BlendFactor::eSrcAlpha,
							.dst_factor = vk::BlendFactor::eOneMinusSrcAlpha,
							.op = vk::BlendOp::eAdd,
						},
						.alpha = {
							.src_factor = vk::BlendFactor::eOne,
							.dst_factor = vk::BlendFactor::eOneMinusSrcAlpha,
							.op = vk::BlendOp::eAdd,
						},
					}},
				},
			},
		}
	);
}

void c3d::BloomPass::createImages()
{
	_workImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Bloom work image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferDst,
			.levels = cgpu::calcImageMaxLevelCount({_size, 1}),
		}
	);

	_outputImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Bloom output image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eSampled,
		}
	);
}

void c3d::BloomPass::createSamplers()
{
	_downsampleSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_v = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_w = vk::SamplerAddressMode::eClampToBorder,
		}
	);

	_upsampleSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_v = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_w = vk::SamplerAddressMode::eClampToEdge,
		}
	);

	_composeSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.wrapping_u = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_v = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_w = vk::SamplerAddressMode::eClampToBorder,
		}
	);
}
