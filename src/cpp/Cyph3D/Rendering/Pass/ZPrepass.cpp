#include "ZPrepass.h"

#include <Cyph3D/Asset/RuntimeAsset/MeshAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Rendering/VertexData.h>
#include <Cyph3D/Scene/Camera.h>
#include <Cyph3D/Scene/Transform.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/VertexInputState.hpp>

c3d::ZPrepass::ZPrepass(glm::uvec2 size):
	RenderPass(size, "Z prepass")
{
	createImage();
	createPipelineStates();
}

c3d::ZPrepassOutput c3d::ZPrepass::onRender(cgpu::CommandRecorder& commandRecorder, ZPrepassInput& input)
{
	commandRecorder.graphicsPass({
		.depth_stencil_attachment = {{
			.image = _multisampledDepthImage,
			.load_op = vk::AttachmentLoadOp::eClear,
			.store_op = vk::AttachmentStoreOp::eStore,
			.clear_depth_value = 1.0f,
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_preRasterizationShaderState,
				_fragmentShaderState,
				_fragmentOutputState
			);

			glm::mat4 vpMatrix = input.camera.getProjection() * input.camera.getView();
			for (const ModelRenderer::RenderData& model : input.registry.getModelRenderRequests())
			{
				ctx.bindIndexBuffer(model.mesh.getIndexBuffer(), model.mesh.getIndexType());

				using namespace cgpu::shader_types;
				struct
				{
					float4x4 u_mvpMatrix{};
					PositionVertexData* u_vertexList{};
				} parameters{};

				parameters.u_mvpMatrix = vpMatrix * model.transform.getLocalToWorldMatrix();
				parameters.u_vertexList = ctx.getBufferDevicePtr<PositionVertexData>(
					model.mesh.getPositionVertexBuffer(),
					cgpu::GraphicsStage::eVertex,
					cgpu::StorageAccess::eReadonly
				);

				ctx.drawIndexed(model.mesh.getIndexCount(), 1, 0, 0, 0, parameters);
			}
		},
	});

	return {
		.multisampledDepthImage = _multisampledDepthImage,
	};
}

void c3d::ZPrepass::onResize()
{
	createImage();
}

void c3d::ZPrepass::createImage()
{
	_multisampledDepthImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Multisampled depth image",
			.format = SceneRenderer::DEPTH_FORMAT,
			.extent = {_size, 1},
			.usages = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.samples = vk::SampleCountFlagBits::e4,
		}
	);
}

void c3d::ZPrepass::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_preRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/z-prepass/z-prepass.slang"},
		}
	);

	_fragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eLess,
				.write_enabled = true,
			}},
		}
	);

	_fragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.depth_stencil_attachment = {{
				.format = SceneRenderer::DEPTH_FORMAT,
			}},
			.samples = vk::SampleCountFlagBits::e4,
		}
	);
}
