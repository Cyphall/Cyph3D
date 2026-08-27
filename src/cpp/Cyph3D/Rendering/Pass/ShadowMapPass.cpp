#include "ShadowMapPass.h"

#include <Cyph3D/Asset/RuntimeAsset/MeshAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Entity/Component/DirectionalLight.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Helper/MathHelper.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Scene/Transform.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/VertexInputState.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace
{
constexpr float POINT_SHADOW_MAP_NEAR = 0.01f;
constexpr float POINT_SHADOW_MAP_FAR = 100.0f;

const glm::mat4 POINT_SHADOW_MAP_PROJECTION = [] {
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, POINT_SHADOW_MAP_NEAR, POINT_SHADOW_MAP_FAR);

	projection[1][1] *= -1;

	return projection;
}();

glm::mat4 calcDirectionalShadowMapView(const c3d::DirectionalLight::RenderData& light)
{
	return glm::lookAt(
		{0, 0, 0},
		light.transform.getDown(),
		light.transform.getForward()
	);
}

std::tuple<glm::mat4, float, float> calcDirectionalShadowMapProjection(const glm::mat4& view, const std::vector<c3d::ModelRenderer::RenderData>& models)
{
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());

	for (const c3d::ModelRenderer::RenderData& model : models)
	{
		glm::mat4 modelView = view * model.transform.getLocalToWorldMatrix();

		auto [boundingBoxMin_SMS, boundingBoxMax_SMS] = c3d::MathHelper::transformBoundingBox(
			modelView,
			model.mesh.getBoundingBoxMin(),
			model.mesh.getBoundingBoxMax()
		);

		min = glm::min(min, boundingBoxMin_SMS);
		max = glm::max(max, boundingBoxMax_SMS);
	}

	min -= 0.01f;
	max += 0.01f;

	glm::vec3 center = (max + min) / 2.0f;

	glm::vec3 size = max - min;
	size.x = size.y = std::max(size.x, size.y); // force the matrix to have the same width and height to get square shadow map pixels in worldspace

	glm::mat4 projection = glm::ortho(
		center.x - size.x / 2, center.x + size.x / 2,
		center.y + size.y / 2, center.y - size.y / 2,
		-center.z - size.z / 2, -center.z + size.z / 2
	);

	return {projection, size.x, size.z};
}

std::array<glm::mat4, 6> calcPointShadowMapView(const c3d::PointLight::RenderData& light)
{
	glm::vec3 position = light.transform.getWorldPosition();

	return {
		glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
		glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
		glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)),
	};
}
}

c3d::ShadowMapPass::ShadowMapPass(glm::uvec2 size):
	RenderPass(size, "Shadow map pass")
{
	createPipelineStates();
}

c3d::ShadowMapPassOutput c3d::ShadowMapPass::onRender(cgpu::CommandRecorder& commandRecorder, ShadowMapPassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_shadowMapManager.resetDirectionalShadowMapAllocations();
		_directionalShadowMapInfos.clear();

		for (const DirectionalLight::RenderData& light : input.registry.getDirectionalLightRenderRequests())
		{
			if (light.castShadows)
			{
				_directionalShadowMapInfos.emplace_back(renderDirectionalShadowMap(commandRecorder, light, input.registry.getModelRenderRequests()));
			}
			else
			{
				_directionalShadowMapInfos.emplace_back(std::nullopt);
			}
		}
	}

	if (input.sceneChanged)
	{
		_shadowMapManager.resetPointShadowMapAllocations();
		_pointShadowMapInfos.clear();

		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (light.castShadows)
			{
				_pointShadowMapInfos.emplace_back(renderPointShadowMap(commandRecorder, light, input.registry.getModelRenderRequests()));
			}
			else
			{
				_pointShadowMapInfos.emplace_back(std::nullopt);
			}
		}
	}

	return {
		.directionalShadowMapInfos = _directionalShadowMapInfos,
		.pointShadowMapInfos = _pointShadowMapInfos,
		.pointLightMaxDistance = POINT_SHADOW_MAP_FAR,
	};
}

void c3d::ShadowMapPass::onResize()
{
}

void c3d::ShadowMapPass::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_directionalPreRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/shadow mapping/directional light.slang"},
		}
	);

	_directionalFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eLess,
				.write_enabled = true,
			}},
		}
	);

	_directionalFragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.depth_stencil_attachment = {{
				.format = SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT,
			}},
		}
	);

	_pointPreRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/shadow mapping/point light.slang"},
		}
	);

	_pointFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/shadow mapping/point light.slang"}},
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eLess,
				.write_enabled = true,
			}},
		}
	);

	_pointFragmentOutputState = cgpu::FragmentOutputState::create(
		Engine::getDeviceSession(),
		{
			.depth_stencil_attachment = {{
				.format = SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT,
			}},
		}
	);
}

c3d::DirectionalShadowMapInfo c3d::ShadowMapPass::renderDirectionalShadowMap(
	cgpu::CommandRecorder& commandRecorder,
	const DirectionalLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models
)
{
	cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("Directional light ({})", light.name)};

	cgpu::ImagePtr shadowMap = _shadowMapManager.allocateDirectionalShadowMap(light.shadowMapResolution);

	glm::mat4 vMatrix = calcDirectionalShadowMapView(light);
	auto [pMatrix, worldSize, worldDepth] = calcDirectionalShadowMapProjection(vMatrix, models);
	glm::mat4 vpMatrix = pMatrix * vMatrix;

	commandRecorder.graphicsPass({
		.depth_stencil_attachment = {{
			.image = shadowMap,
			.load_op = vk::AttachmentLoadOp::eClear,
			.store_op = vk::AttachmentStoreOp::eStore,
			.clear_depth_value = 1.0f,
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			ctx.bindPipelineStates(
				_vertexInputState,
				_directionalPreRasterizationShaderState,
				_directionalFragmentShaderState,
				_directionalFragmentOutputState
			);

			for (const ModelRenderer::RenderData& model : models)
			{
				if (!model.contributeShadows)
				{
					continue;
				}

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
		.worldSize = worldSize,
		.worldDepth = worldDepth,
		.vpMatrix = vpMatrix,
		.image = shadowMap,
	};
}

c3d::PointShadowMapInfo c3d::ShadowMapPass::renderPointShadowMap(
	cgpu::CommandRecorder& commandRecorder,
	const PointLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models
)
{
	cgpu::ScopedDebugRegion debugRegion{commandRecorder, std::format("Point light ({})", light.name)};

	cgpu::ImagePtr shadowMap = _shadowMapManager.allocatePointShadowMap(light.shadowMapResolution);

	std::array<glm::mat4, 6> views = calcPointShadowMapView(light);

	for (int i = 0; i < 6; i++)
	{
		commandRecorder.graphicsPass({
			.depth_stencil_attachment = {{
				.image = shadowMap,
				.first_layer = i,
				.load_op = vk::AttachmentLoadOp::eClear,
				.store_op = vk::AttachmentStoreOp::eStore,
				.clear_depth_value = 1.0f,
			}},
			.callback = [&](cgpu::GraphicsPassContext& ctx) {
				ctx.bindPipelineStates(
					_vertexInputState,
					_pointPreRasterizationShaderState,
					_pointFragmentShaderState,
					_pointFragmentOutputState
				);

				glm::mat4 vpMatrix = POINT_SHADOW_MAP_PROJECTION * views[i];
				for (const ModelRenderer::RenderData& model : models)
				{
					if (!model.contributeShadows)
					{
						continue;
					}

					ctx.bindIndexBuffer(model.mesh.getIndexBuffer(), model.mesh.getIndexType());

					using namespace cgpu::shader_types;
					struct
					{
						float4x4 u_mvpMatrix;
						float4x4 u_mMatrix;
						PositionVertexData* u_vertexList;
						float3 u_lightPos;
						float u_invMaxDistance;
					} parameters{};

					parameters.u_mvpMatrix = vpMatrix * model.transform.getLocalToWorldMatrix();
					parameters.u_mMatrix = model.transform.getLocalToWorldMatrix();
					parameters.u_vertexList = ctx.getBufferDevicePtr<PositionVertexData>(
						model.mesh.getPositionVertexBuffer(),
						cgpu::GraphicsStage::eVertex,
						cgpu::StorageAccess::eReadonly
					);
					parameters.u_lightPos = light.transform.getWorldPosition();
					parameters.u_invMaxDistance = 1.0f / POINT_SHADOW_MAP_FAR;

					ctx.drawIndexed(model.mesh.getIndexCount(), 1, 0, 0, 0, parameters);
				}
			},
		});
	}

	return {
		.image = shadowMap,
	};
}
