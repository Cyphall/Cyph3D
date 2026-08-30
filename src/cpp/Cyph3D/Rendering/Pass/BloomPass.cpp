#include "BloomPass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>
#include <CyphGPU/Sampler.hpp>

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
	_workImages[0] = input.lightImage;

	for (size_t i = 0; i < _workImages.size() - 1; i++)
	{
		uint32_t srcLevel = i + 0;
		uint32_t dstLevel = i + 1;
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("downsample({}->{})", srcLevel, dstLevel)};
		downsample(commandRecorder, srcLevel, dstLevel);
	}

	// upsample and blur work image
	for (size_t i = 0; i < _workImages.size() - 1; i++)
	{
		uint32_t srcLevel = _workImages.size() - 1 - i;
		uint32_t dstLevel = _workImages.size() - 2 - i;
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("upsample({}->{})", srcLevel, dstLevel)};
		upsample(commandRecorder, srcLevel, dstLevel);
	}

	_workImages[0] = nullptr;

	return {
		.lightImage = input.lightImage,
	};
}

void c3d::BloomPass::onResize()
{
	createImages();
}

void c3d::BloomPass::downsample(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				Texture2D<>::Handle u_srcImage;
				SamplerState::Handle u_sampler;
				WTexture2D<>::Handle u_dstImage;
				float2 u_srcPixelSize;
			} parameters{};

			parameters.u_size = glm::uvec2{_workImages[dstLevel]->getDesc().extent};
			parameters.u_srcImage = ctx.getSampledImageDescriptor(_workImages[srcLevel]);
			parameters.u_sampler = _downsampleSampler->getDescriptor();
			parameters.u_dstImage = ctx.getStorageImageDescriptor(_workImages[dstLevel], cgpu::StorageAccess::eWriteonly);
			parameters.u_srcPixelSize = glm::vec2{1.0f} / glm::vec2{_workImages[srcLevel]->getDesc().extent};

			ctx.dispatch(_downsampleShaderState, {parameters.u_size.get(), 1}, {8, 8, 1}, parameters);
		},
	});
}

void c3d::BloomPass::upsample(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				Texture2D<>::Handle u_srcImage;
				SamplerState::Handle u_sampler;
				RWTexture2D<>::Handle u_dstImage;
				float2 u_srcPixelSize;
				float u_weight;
			} parameters{};

			parameters.u_size = glm::uvec2{_workImages[dstLevel]->getDesc().extent};
			parameters.u_srcImage = ctx.getSampledImageDescriptor(_workImages[srcLevel]);
			parameters.u_sampler = _upsampleSampler->getDescriptor();
			parameters.u_dstImage = ctx.getStorageImageDescriptor(_workImages[dstLevel], cgpu::StorageAccess::eReadWrite);
			parameters.u_srcPixelSize = glm::vec2{1.0f} / glm::vec2{_workImages[srcLevel]->getDesc().extent};
			parameters.u_weight = dstLevel == 0 ? BLOOM_STRENGTH : BLOOM_RADIUS;

			ctx.dispatch(_upsampleShaderState, {parameters.u_size.get(), 1}, {8, 8, 1}, parameters);
		},
	});
}

void c3d::BloomPass::createPipelineStates()
{
	_downsampleShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/bloom/downsample.slang"},
		}
	);

	_upsampleShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/bloom/upsample.slang"},
		}
	);
}

void c3d::BloomPass::createImages()
{
	// _workImages[0] will be the input/output image
	uint32_t levelCount = cgpu::calcImageMaxLevelCount({_size, 1});
	_workImages.resize(levelCount);
	for (uint32_t level = 1; level < levelCount; level++)
	{
		_workImages[level] = cgpu::Image::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("Bloom work image level {}", level),
				.format = SceneRenderer::HDR_COLOR_FORMAT,
				.extent = cgpu::calcImageLevelExtent({_size, 1}, level),
				.usages =
					vk::ImageUsageFlagBits::eSampled |
					vk::ImageUsageFlagBits::eStorage,
			}
		);
	}
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
}
