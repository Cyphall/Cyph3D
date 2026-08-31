#include "LightingPass.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Asset/RuntimeAsset/MaterialAsset.h>
#include <Cyph3D/Asset/RuntimeAsset/MeshAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Helper/MathHelper.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Scene/Camera.h>
#include <Cyph3D/Scene/Scene.h>
#include <Cyph3D/Scene/Transform.h>

#include <CyphGPU/FragmentOutputState.hpp>
#include <CyphGPU/FragmentShaderState.hpp>
#include <CyphGPU/GraphicsPassContext.hpp>
#include <CyphGPU/PreRasterizationShaderState.hpp>
#include <CyphGPU/Sampler.hpp>
#include <CyphGPU/VertexInputState.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

namespace
{
using namespace cgpu::shader_types;

struct GPUDirectionalLight
{
	float3 fragToLightDirection{};
	float intensity{};
	float3 color{};
	bool castShadows{};
	float4x4 vpMatrix{};
	Texture2D<>::Handle image{};
	float shadowMapTexelWorldSize{};
};

struct GPUPointLight
{
	float3 pos{};
	float intensity{};
	float3 color{};
	bool castShadows{};
	TextureCube<>::Handle image{};
	float maxTexelSizeAtUnitDistance{};
};
}

c3d::LightingPass::LightingPass(glm::uvec2 size):
	RenderPass(size, "Lighting pass")
{
	createImage();
	createSamplers();
	createPipelineStates();
}

c3d::LightingPassOutput c3d::LightingPass::onRender(cgpu::CommandRecorder& commandRecorder, LightingPassInput& input)
{
	commandRecorder.graphicsPass({
		.color_attachments = {{
			{
				.image = _multisampledLightImage,
				.load_op = vk::AttachmentLoadOp::eClear,
				.store_op = vk::AttachmentStoreOp::eDontCare,
				.clear_color_value = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
				.resolve = {{
					.image = _lightImage,
				}},
			},
		}},
		.depth_stencil_attachment = {{
			.image = input.multisampledDepthImage,
			.load_op = vk::AttachmentLoadOp::eLoad,
			.store_op = vk::AttachmentStoreOp::eNone,
		}},
		.callback = [&](cgpu::GraphicsPassContext& ctx) {
			{
				std::optional<cgpu::BufferPtr> directionalLightBuffer;
				if (!input.registry.getDirectionalLightRenderRequests().empty())
				{
					directionalLightBuffer = cgpu::Buffer::create(
						Engine::getDeviceSession(),
						{
							.name = "Directional light buffer",
							.size = input.registry.getDirectionalLightRenderRequests().size() * sizeof(GPUDirectionalLight),
							.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
							.min_alignment = alignof(GPUDirectionalLight),
						}
					);

					GPUDirectionalLight* directionalLightPtr = (*directionalLightBuffer)->getHostPtr<GPUDirectionalLight>();
					for (int i = 0; i < input.registry.getDirectionalLightRenderRequests().size(); i++)
					{
						const DirectionalLight::RenderData& light = input.registry.getDirectionalLightRenderRequests()[i];

						directionalLightPtr[i].fragToLightDirection = light.transform.getUp();
						directionalLightPtr[i].intensity = light.intensity;
						directionalLightPtr[i].color = light.color;
						directionalLightPtr[i].castShadows = light.castShadows;
						if (light.castShadows)
						{
							const DirectionalShadowMapInfo& shadowMapInfo = *input.directionalShadowMapInfos[i];

							directionalLightPtr[i].vpMatrix = shadowMapInfo.vpMatrix;
							directionalLightPtr[i].image = ctx.getSampledImageDescriptor(shadowMapInfo.image, cgpu::GraphicsStage::eFragment);
							directionalLightPtr[i].shadowMapTexelWorldSize = shadowMapInfo.worldSize / static_cast<float>(light.shadowMapResolution);
						}
					}
				}

				std::optional<cgpu::BufferPtr> pointLightBuffer;
				if (!input.registry.getPointLightRenderRequests().empty())
				{
					pointLightBuffer = cgpu::Buffer::create(
						Engine::getDeviceSession(),
						{
							.name = "Point light buffer",
							.size = input.registry.getPointLightRenderRequests().size() * sizeof(GPUPointLight),
							.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
							.min_alignment = alignof(GPUPointLight),
						}
					);

					GPUPointLight* pointLightPtr = (*pointLightBuffer)->getHostPtr<GPUPointLight>();
					for (int i = 0; i < input.registry.getPointLightRenderRequests().size(); i++)
					{
						const PointLight::RenderData& light = input.registry.getPointLightRenderRequests()[i];

						pointLightPtr[i].pos = light.transform.getWorldPosition();
						pointLightPtr[i].intensity = light.intensity;
						pointLightPtr[i].color = light.color;
						pointLightPtr[i].castShadows = light.castShadows;
						if (light.castShadows)
						{
							const PointShadowMapInfo& shadowMapInfo = *input.pointShadowMapInfos[i];

							pointLightPtr[i].image = ctx.getSampledImageDescriptor(shadowMapInfo.image, cgpu::GraphicsStage::eFragment, {.type = vk::ImageViewType::eCube});
							pointLightPtr[i].maxTexelSizeAtUnitDistance = 2.0f / static_cast<float>(light.shadowMapResolution);
						}
					}
				}

				ctx.bindPipelineStates(
					_vertexInputState,
					_objectPreRasterizationShaderState,
					_objectFragmentShaderState,
					_fragmentOutputState
				);

				using namespace cgpu::shader_types;
				struct
				{
					GPUDirectionalLight* u_directionalLightList{};
					GPUPointLight* u_pointLightList{};
					SamplerState::Handle u_materialSampler{};
					SamplerComparisonState::Handle u_directionalLightSampler{};
					SamplerComparisonState::Handle u_pointLightSampler{};
					float3 u_viewPos{};
					uint32_t u_frameIndex{};
					uint32_t u_directionalLightCount{};
					uint32_t u_pointLightCount{};
					float u_pointLightMaxDistance{};

					float3x3 u_normalMatrix{};
					float4x4 u_mMatrix{};
					float4x4 u_mvpMatrix{};
					PositionVertexData* u_positionVertexList{};
					MaterialVertexData* u_materialVertexList{};
					Texture2D<>::Handle u_albedoImage{};
					Texture2D<>::Handle u_normalImage{};
					Texture2D<>::Handle u_roughnessImage{};
					Texture2D<>::Handle u_metalnessImage{};
					Texture2D<>::Handle u_displacementImage{};
					Texture2D<>::Handle u_emissiveImage{};
					float3 u_albedoValue{};
					float u_roughnessValue{};
					float u_metalnessValue{};
					float u_displacementScale{};
					float u_emissiveScale{};
				} parameters{};

				parameters.u_directionalLightList =
					directionalLightBuffer ?
						ctx.getBufferDevicePtr<GPUDirectionalLight>(*directionalLightBuffer, cgpu::GraphicsStage::eFragment, cgpu::StorageAccess::eReadonly) :
						nullptr;
				parameters.u_pointLightList =
					pointLightBuffer ?
						ctx.getBufferDevicePtr<GPUPointLight>(*pointLightBuffer, cgpu::GraphicsStage::eFragment, cgpu::StorageAccess::eReadonly) :
						nullptr;
				parameters.u_materialSampler = Engine::getAssetManager().getTextureSampler()->getDescriptor();
				parameters.u_directionalLightSampler = _directionalLightSampler->getDescriptor();
				parameters.u_pointLightSampler = _pointLightSampler->getDescriptor();
				parameters.u_viewPos = input.camera.getPosition();
				parameters.u_frameIndex = _frameIndex;
				parameters.u_directionalLightCount = static_cast<uint32_t>(input.registry.getDirectionalLightRenderRequests().size());
				parameters.u_pointLightCount = static_cast<uint32_t>(input.registry.getPointLightRenderRequests().size());
				parameters.u_pointLightMaxDistance = input.pointLightMaxDistance;

				glm::mat4 vpMatrix = input.camera.getProjection() * input.camera.getView();
				for (const ModelRenderer::RenderData& model : input.registry.getModelRenderRequests())
				{
					ctx.bindIndexBuffer(model.mesh.getIndexBuffer(), model.mesh.getIndexType());

					auto albedoImage = model.material.getAlbedoImage();
					auto normalImage = model.material.getNormalImage();
					auto roughnessImage = model.material.getRoughnessImage();
					auto metalnessImage = model.material.getMetalnessImage();
					auto displacementImage = model.material.getDisplacementImage();
					auto emissiveImage = model.material.getEmissiveImage();

					parameters.u_normalMatrix = glm::inverseTranspose(glm::mat3(model.transform.getLocalToWorldMatrix()));
					parameters.u_mMatrix = model.transform.getLocalToWorldMatrix();
					parameters.u_mvpMatrix = vpMatrix * model.transform.getLocalToWorldMatrix();
					parameters.u_positionVertexList = ctx.getBufferDevicePtr<PositionVertexData>(
						model.mesh.getPositionVertexBuffer(),
						cgpu::GraphicsStage::eVertex,
						cgpu::StorageAccess::eReadonly
					);
					parameters.u_materialVertexList = ctx.getBufferDevicePtr<MaterialVertexData>(
						model.mesh.getMaterialVertexBuffer(),
						cgpu::GraphicsStage::eVertex,
						cgpu::StorageAccess::eReadonly
					);
					parameters.u_albedoImage = albedoImage ? ctx.getSampledImageDescriptor(*albedoImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_normalImage = normalImage ? ctx.getSampledImageDescriptor(*normalImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_roughnessImage = roughnessImage ? ctx.getSampledImageDescriptor(*roughnessImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_metalnessImage = metalnessImage ? ctx.getSampledImageDescriptor(*metalnessImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_displacementImage = displacementImage ? ctx.getSampledImageDescriptor(*displacementImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_emissiveImage = emissiveImage ? ctx.getSampledImageDescriptor(*emissiveImage, cgpu::GraphicsStage::eFragment) : nullptr;
					parameters.u_albedoValue = MathHelper::srgbToLinear(model.material.getAlbedoValue());
					parameters.u_roughnessValue = model.material.getRoughnessValue();
					parameters.u_metalnessValue = model.material.getMetalnessValue();
					parameters.u_displacementScale = model.material.getDisplacementScale();
					parameters.u_emissiveScale = model.material.getEmissiveScale();

					ctx.drawIndexed(model.mesh.getIndexCount(), 1, 0, 0, 0, parameters);
				}
			}

			if (Engine::getScene().getSkybox() != nullptr && Engine::getScene().getSkybox()->isLoaded())
			{
				ctx.bindPipelineStates(
					_vertexInputState,
					_skyboxPreRasterizationShaderState,
					_skyboxFragmentShaderState,
					_fragmentOutputState
				);

				using namespace cgpu::shader_types;
				struct
				{
					float4x4 u_invMvpMatrix;
					TextureCube<>::Handle u_image;
					SamplerState::Handle u_sampler;
				} parameters{};

				parameters.u_invMvpMatrix = glm::inverse(
					input.camera.getProjection() *
					glm::mat4(glm::mat3(input.camera.getView())) *
					glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0))
				);
				parameters.u_image = ctx.getSampledImageDescriptor(
					Engine::getScene().getSkybox()->getCubemap()->getImage(),
					cgpu::GraphicsStage::eFragment,
					{.type = vk::ImageViewType::eCube}
				);
				parameters.u_sampler = Engine::getAssetManager().getCubemapSampler()->getDescriptor();

				ctx.draw(3, 1, 0, 0, parameters);
			}
		},
	});

	_frameIndex++;

	return {
		.lightImage = _lightImage,
	};
}

void c3d::LightingPass::onResize()
{
	createImage();
}

void c3d::LightingPass::createSamplers()
{
	_directionalLightSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_v = vk::SamplerAddressMode::eClampToBorder,
			.wrapping_w = vk::SamplerAddressMode::eClampToBorder,
			.comparison_mode = vk::CompareOp::eGreater,
			.border_color = vk::BorderColor::eFloatOpaqueWhite,
		}
	);

	_pointLightSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_v = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_w = vk::SamplerAddressMode::eClampToEdge,
			.comparison_mode = vk::CompareOp::eGreater,
			.border_color = vk::BorderColor::eFloatOpaqueWhite,
		}
	);
}

void c3d::LightingPass::createPipelineStates()
{
	_vertexInputState = cgpu::VertexInputState::create(
		Engine::getDeviceSession(),
		{}
	);

	_objectPreRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/lighting/object.slang"},
		}
	);

	_objectFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/lighting/object.slang"}},
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eEqual,
				.write_enabled = false,
			}},
		}
	);

	_skyboxPreRasterizationShaderState = cgpu::PreRasterizationShaderState::create(
		Engine::getDeviceSession(),
		{
			.vertex_shader = {.source = "Cyph3D/lighting/skybox.slang"},
		}
	);

	_skyboxFragmentShaderState = cgpu::FragmentShaderState::create(
		Engine::getDeviceSession(),
		{
			.fragment_shader = {{.source = "Cyph3D/lighting/skybox.slang"}},
			.depth_state = {{
				.test_pass_condition = vk::CompareOp::eEqual,
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

void c3d::LightingPass::createImage()
{
	_multisampledLightImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Multisampled light image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eColorAttachment,
			.samples = vk::SampleCountFlagBits::e4,
		}
	);

	_lightImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Light image",
			.format = SceneRenderer::HDR_COLOR_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eStorage,
		}
	);
}
