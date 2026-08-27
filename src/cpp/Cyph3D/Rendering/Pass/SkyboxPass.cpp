#include "SkyboxPass.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Asset/RuntimeAsset/CubemapAsset.h>
#include <Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Scene/Camera.h>
#include <Cyph3D/Scene/Scene.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/Sampler.hpp>
#include <CyphGPU/VertexInputState.hpp>
#include <glm/gtx/transform.hpp>

c3d::SkyboxPass::SkyboxPass(glm::uvec2 size):
	RenderPass(size, "Skybox pass")
{
	createPipelineStates();
	createImages();
	createBuffer();
}

c3d::SkyboxPassOutput c3d::SkyboxPass::onRender(cgpu::CommandRecorder& commandRecorder, SkyboxPassInput& input)
{
	if (Engine::getScene().getSkybox() != nullptr && Engine::getScene().getSkybox()->isLoaded())
	{
		commandRecorder.graphicsPass({
			.color_attachments = {{
				{
					.image = input.multisampledLightImage,
					.load_op = vk::AttachmentLoadOp::eLoad,
					.store_op = vk::AttachmentStoreOp::eStore,
				},
			}},
			.depth_stencil_attachment = {{
				.image = input.multisampledDepthImage,
				.load_op = vk::AttachmentLoadOp::eLoad,
				.store_op = vk::AttachmentStoreOp::eDontCare,
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
					float4x4 u_mvpMatrix;
					float3* u_positionList;
					TextureCube<>::Handle u_image;
					SamplerState::Handle u_sampler;
				} parameters{};

				parameters.u_mvpMatrix =
					input.camera.getProjection() *
					glm::mat4(glm::mat3(input.camera.getView())) *
					glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
				parameters.u_positionList = ctx.getBufferDevicePtr<float3>(
					_vertexBuffer,
					cgpu::GraphicsStage::eVertex,
					cgpu::StorageAccess::eReadonly
				);
				parameters.u_image = ctx.getSampledImageDescriptor(
					Engine::getScene().getSkybox()->getCubemap()->getImage(),
					cgpu::GraphicsStage::eFragment,
					{.type = vk::ImageViewType::eCube}
				);
				parameters.u_sampler = Engine::getAssetManager().getCubemapSampler()->getDescriptor();

				ctx.draw(_vertexBuffer->getDesc().size / sizeof(float3), 1, 0, 0, parameters);
			},
		});
	}

	//TODO: switch back to in-pass resolve when Nvidia driver is fixed
	commandRecorder.resolve({
		.src_image = input.multisampledLightImage,
		.dst_image = _lightImage,
	});

	return {
		.lightImage = _lightImage,
	};
}

void c3d::SkyboxPass::onResize()
{
	createImages();
}

void c3d::SkyboxPass::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_preRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/skybox/skybox.slang"},
		}
	);

	_fragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/skybox/skybox.slang"}},
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eLessOrEqual,
				.write_enabled = false,
			}},
		}
	);

	_fragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.color_attachments = {
				{
					.format = SceneRenderer::HDR_COLOR_FORMAT,
				},
			},
			.depth_stencil_attachment = {{
				.format = SceneRenderer::DEPTH_FORMAT,
			}},
			.samples = vk::SampleCountFlagBits::e4,
		}
	);
}

void c3d::SkyboxPass::createImages()
{
	_lightImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Light image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled,
		}
	);
}

void c3d::SkyboxPass::createBuffer()
{
	std::vector<cgpu::shader_types::float3> vertices = {
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},

		{{-1.0f, -1.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f}},

		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},

		{{-1.0f, -1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f}},

		{{-1.0f, 1.0f, -1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f}},

		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}}
	};

	_vertexBuffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = "Skybox vertex buffer",
			.size = vertices.size() * sizeof(cgpu::shader_types::float3),
			.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
			.min_alignment = alignof(cgpu::shader_types::float3),
		}
	);

	std::ranges::copy(vertices, _vertexBuffer->getHostPtr<cgpu::shader_types::float3>());
}
