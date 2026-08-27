#include "ObjectPicker.h"

#include <Cyph3D/Asset/RuntimeAsset/MeshAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Entity/Component/ModelRenderer.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/VertexData.h>
#include <Cyph3D/Scene/Camera.h>
#include <Cyph3D/Scene/Transform.h>

#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/Image.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/VertexInputState.hpp>

c3d::ObjectPicker::ObjectPicker():
	_commandContext{Engine::getDeviceSession()}
{
	createPipelineStates();
}

c3d::ObjectPicker::~ObjectPicker() = default;

c3d::Entity* c3d::ObjectPicker::getPickedEntity(const Camera& camera, const RenderRegistry& renderRegistry, const glm::uvec2& viewportSize, const glm::uvec2& clickPos)
{
	if (viewportSize.x * viewportSize.y == 0)
	{
		return nullptr;
	}

	if (viewportSize != _currentSize)
	{
		_currentSize = viewportSize;
		createImages();
	}

	auto commandRecorder = _commandContext.createRecorder(Engine::getDeviceSession()->getMainQueue());

	commandRecorder.graphicsPass({
		.color_attachments = {{
			{
				.image = _objectIndexImage,
				.load_op = vk::AttachmentLoadOp::eClear,
				.store_op = vk::AttachmentStoreOp::eStore,
				.clear_color_value = glm::ivec4{-1, 0, 0, 0},
			},
		}},
		.depth_stencil_attachment = {{
			.image = _depthImage,
			.load_op = vk::AttachmentLoadOp::eClear,
			.store_op = vk::AttachmentStoreOp::eDontCare,
			.clear_depth_value = 1.0f,
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_preRasterizationShaderState,
				_fragmentShaderState,
				_fragmentOutputState
			);

			glm::mat4 vpMatrix = camera.getProjection() * camera.getView();
			for (int i = 0; i < renderRegistry.getModelRenderRequests().size(); i++)
			{
				const auto& model = renderRegistry.getModelRenderRequests()[i];

				ctx.bindIndexBuffer(model.mesh.getIndexBuffer(), model.mesh.getIndexType());

				using namespace cgpu::shader_types;
				struct
				{
					float4x4 u_mvpMatrix{};
					PositionVertexData* u_vertexList{};
					int32_t u_objectIndex{};
				} parameters{};

				parameters.u_mvpMatrix = vpMatrix * model.transform.getLocalToWorldMatrix();
				parameters.u_vertexList = ctx.getBufferDevicePtr<PositionVertexData>(
					model.mesh.getPositionVertexBuffer(),
					cgpu::GraphicsStage::eVertex,
					cgpu::StorageAccess::eReadonly
				);
				parameters.u_objectIndex = i;

				ctx.drawIndexed(model.mesh.getIndexCount(), 1, 0, 0, 0, parameters);
			}
		},
	});

	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = "Object picker staging buffer",
			.size = sizeof(int32_t),
			.usages = vk::BufferUsageFlagBits2::eTransferDst,
			.memory_type = cgpu::MemoryType::eCPUCached,
			.min_alignment = alignof(int32_t),
		}
	);

	commandRecorder.copyImageToBuffer({
		.src_image = _objectIndexImage,
		.dst_buffer = stagingBuffer,
		.ranges = {{
			{
				.src = {{
					.pixels = {{
						.offset = {clickPos, 0},
						.size = {1, 1, 1},
					}},
				}},
			},
		}},
	});

	commandRecorder.submit().waitFinished();

	_commandContext.finish();

	int objectIndex = *stagingBuffer->getHostPtr<int32_t>();

	return objectIndex != -1 ? &renderRegistry.getModelRenderRequests()[objectIndex].owner : nullptr;
}

void c3d::ObjectPicker::createImages()
{
	_objectIndexImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Object picker object index image",
			.format = vk::Format::eR32Sint,
			.extent = {_currentSize, 1},
			.usages =
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eTransferSrc,
		}
	);

	_depthImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Object picker depth image",
			.format = vk::Format::eD32Sfloat,
			.extent = {_currentSize, 1},
			.usages = vk::ImageUsageFlagBits::eDepthStencilAttachment,
		}
	);
}

void c3d::ObjectPicker::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_preRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/object picker/object picker.slang"},
		}
	);

	_fragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/object picker/object picker.slang"}},
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eLess,
				.write_enabled = true,
			}},
		}
	);

	_fragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.color_attachments = {
				{
					.format = vk::Format::eR32Sint,
				},
			},
			.depth_stencil_attachment = {{
				.format = vk::Format::eD32Sfloat,
			}},
		}
	);
}
