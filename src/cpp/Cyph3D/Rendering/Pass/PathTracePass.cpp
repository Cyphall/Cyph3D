#include "PathTracePass.h"

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

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>
#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/TLAS.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

namespace
{
using namespace cgpu::shader_types;

struct GPUInstance
{
	float3x3 normalMatrix{};
	c3d::PositionVertexData* positionVertexBuffer{};
	c3d::MaterialVertexData* materialVertexBuffer{};
	uint32_t* indexBuffer{};
	Texture2D<>::Handle albedoImage{};
	Texture2D<>::Handle normalImage{};
	Texture2D<>::Handle roughnessImage{};
	Texture2D<>::Handle metalnessImage{};
	Texture2D<>::Handle displacementImage{};
	Texture2D<>::Handle emissiveImage{};
	float3 albedoValue{};
	float roughnessValue{};
	float metalnessValue{};
	float emissiveScale{};
};
}

c3d::PathTracePass::PathTracePass(const glm::uvec2& size):
	RenderPass(size, "Path trace pass")
{
	createPipelineState();
	createImages();
}

c3d::PathTracePassOutput c3d::PathTracePass::onRender(cgpu::CommandRecorder& commandRecorder, PathTracePassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_accumulatedSamples = 0;
	}

	if (input.sceneChanged)
	{
		recreateTLAS(commandRecorder, input);

		// Workaround: Sync is broken on Nvidia driver 616.56, presumably
		// when VkDependencyInfo::memoryBarrierCount, bufferMemoryBarrierCount and
		// imageMemoryBarrierCount are all 0 and the only barriers are specified
		// in VkMemoryRangeBarriersInfoKHR::memoryRangeBarrierCount
		commandRecorder.debugBarrier({
			.src_stages = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			.src_accesses = vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
			.dst_stages = vk::PipelineStageFlagBits2::eComputeShader,
			.dst_accesses = vk::AccessFlagBits2::eAccelerationStructureReadKHR,
		});
	}

	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			std::optional<cgpu::BufferPtr> instanceBuffer;
			if (!input.registry.getModelRenderRequests().empty())
			{
				instanceBuffer = cgpu::Buffer::create(
					Engine::getDeviceSession(),
					{
						.name = "Instance buffer",
						.size = input.registry.getModelRenderRequests().size() * sizeof(GPUInstance),
						.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
						.min_alignment = alignof(GPUInstance),
					}
				);

				GPUInstance* instancePtr = (*instanceBuffer)->getHostPtr<GPUInstance>();
				for (int i = 0; i < input.registry.getModelRenderRequests().size(); i++)
				{
					const ModelRenderer::RenderData& model = input.registry.getModelRenderRequests()[i];

					auto albedoImage = model.material.getAlbedoImage();
					auto normalImage = model.material.getNormalImage();
					auto roughnessImage = model.material.getRoughnessImage();
					auto metalnessImage = model.material.getMetalnessImage();
					auto displacementImage = model.material.getDisplacementImage();
					auto emissiveImage = model.material.getEmissiveImage();

					instancePtr[i].normalMatrix = glm::inverseTranspose(glm::mat3(model.transform.getLocalToWorldMatrix()));
					instancePtr[i].positionVertexBuffer = ctx.getBufferDevicePtr<PositionVertexData>(model.mesh.getPositionVertexBuffer(), cgpu::StorageAccess::eReadonly);
					instancePtr[i].materialVertexBuffer = ctx.getBufferDevicePtr<MaterialVertexData>(model.mesh.getMaterialVertexBuffer(), cgpu::StorageAccess::eReadonly);
					instancePtr[i].indexBuffer = ctx.getBufferDevicePtr<uint32_t>(model.mesh.getIndexBuffer(), cgpu::StorageAccess::eReadonly);
					instancePtr[i].albedoImage = albedoImage ? ctx.getSampledImageDescriptor(*albedoImage) : nullptr;
					instancePtr[i].normalImage = normalImage ? ctx.getSampledImageDescriptor(*normalImage) : nullptr;
					instancePtr[i].roughnessImage = roughnessImage ? ctx.getSampledImageDescriptor(*roughnessImage) : nullptr;
					instancePtr[i].metalnessImage = metalnessImage ? ctx.getSampledImageDescriptor(*metalnessImage) : nullptr;
					instancePtr[i].displacementImage = displacementImage ? ctx.getSampledImageDescriptor(*displacementImage) : nullptr;
					instancePtr[i].emissiveImage = emissiveImage ? ctx.getSampledImageDescriptor(*emissiveImage) : nullptr;
					instancePtr[i].albedoValue = MathHelper::srgbToLinear(model.material.getAlbedoValue());
					instancePtr[i].roughnessValue = model.material.getRoughnessValue();
					instancePtr[i].metalnessValue = model.material.getMetalnessValue();
					instancePtr[i].emissiveScale = model.material.getEmissiveScale();
				}
			}

			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				uint64_t u_tlas;
				GPUInstance* u_instanceBuffer;
				SamplerState::Handle u_sampler;
				SamplerState::Handle u_skyboxSampler;
				RWTexture2D<>::Handle u_lightImage;
				TextureCube<>::Handle u_skyboxImage;
				float3x3 u_skyboxRotation;
				float3 u_camPosition;
				float3 u_camRayTL;
				float3 u_camRayTR;
				float3 u_camRayBL;
				float3 u_camRayBR;
				uint32_t u_batchIndex;
				uint32_t u_sampleCount;
				bool u_resetAccumulation;
			} parameters{};

			SkyboxAsset* skybox = Engine::getScene().getSkybox();

			parameters.u_size = _size;
			parameters.u_tlas = ctx.getTLASDevicePtr(_tlas);
			parameters.u_instanceBuffer = instanceBuffer ? ctx.getBufferDevicePtr<GPUInstance>(*instanceBuffer, cgpu::StorageAccess::eReadonly) : nullptr;
			parameters.u_sampler = Engine::getAssetManager().getTextureSampler()->getDescriptor();
			parameters.u_skyboxSampler = Engine::getAssetManager().getCubemapSampler()->getDescriptor();
			parameters.u_lightImage = ctx.getStorageImageDescriptor(_lightImage, cgpu::StorageAccess::eReadWrite);
			if (skybox && skybox->isLoaded())
			{
				parameters.u_skyboxImage = ctx.getSampledImageDescriptor(skybox->getCubemap()->getImage(), {.type = vk::ImageViewType::eCube});
				parameters.u_skyboxRotation = glm::mat3{glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3{0, 1, 0})};
			}
			parameters.u_camPosition = input.camera.getPosition();
			parameters.u_camRayTL = input.camera.getCornerRays()[0];
			parameters.u_camRayTR = input.camera.getCornerRays()[1];
			parameters.u_camRayBL = input.camera.getCornerRays()[2];
			parameters.u_camRayBR = input.camera.getCornerRays()[3];
			parameters.u_batchIndex = _batchIndex;
			parameters.u_sampleCount = input.sampleCount;
			parameters.u_resetAccumulation = _accumulatedSamples == 0;

			ctx.dispatch(_computeShaderState, {_size, 1}, {8, 8, 1}, parameters);
		},
	});

	_accumulatedSamples += input.sampleCount;
	_batchIndex++;

	return {
		.lightImage = _lightImage,
		.accumulatedSamples = _accumulatedSamples,
	};
}

void c3d::PathTracePass::onResize()
{
	createImages();
	_accumulatedSamples = 0;
}

void c3d::PathTracePass::recreateTLAS(cgpu::CommandRecorder& commandRecorder, const PathTracePassInput& input)
{
	cgpu::TLAS::ASInfo tlasInfo{
		.instance_count = static_cast<uint32_t>(input.registry.getModelRenderRequests().size()),
	};

	auto sizes = cgpu::TLAS::calcSizes(Engine::getDeviceSession(), tlasInfo);

	cgpu::BufferPtr tlas_buffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = "TLAS buffer",
			.size = sizes.accelerationStructureSize,
			.usages = vk::BufferUsageFlagBits2::eAccelerationStructureStorageKHR,
			.min_alignment = 256,
		}
	);

	_tlas = cgpu::TLAS::create(
		Engine::getDeviceSession(),
		{
			.name = "TLAS",
			.as_info = tlasInfo,
			.buffer = tlas_buffer,
			.sizes = sizes,
		}
	);

	std::optional<cgpu::CommandRecorder::TLASParams::InstanceInfo> tlas_instance_info;
	if (!input.registry.getModelRenderRequests().empty())
	{
		tlas_instance_info = {{
			.data = {{}},
			.buffer = cgpu::Buffer::create(
				Engine::getDeviceSession(),
				{
					.name = "TLAS (build instance memory)",
					.size = input.registry.getModelRenderRequests().size() * sizeof(vk::AccelerationStructureInstanceKHR),
					.usages = vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR,
					.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
					.min_alignment = 16,
				}
			),
		}};

		for (int i = 0; i < input.registry.getModelRenderRequests().size(); i++)
		{
			const ModelRenderer::RenderData& model = input.registry.getModelRenderRequests()[i];

			tlas_instance_info->data->push_back({
				.blas = model.mesh.getBLAS(),
				.local_to_world = model.transform.getLocalToWorldMatrix(),
			});
		}
	}

	std::optional<cgpu::CommandRecorder::TLASParams::ScratchBuffer> tlas_scratch_buffer;
	if (_tlas->getDesc().sizes.buildScratchSize > 0)
	{
		tlas_scratch_buffer = {{
			.buffer = cgpu::Buffer::create(
				Engine::getDeviceSession(),
				{
					.name = "TLAS (build scratch memory)",
					.size = _tlas->getDesc().sizes.buildScratchSize,
					.usages = vk::BufferUsageFlagBits2::eStorageBuffer,
					.min_alignment = Engine::getDeviceSession()->getDevice()->getProperties<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>().minAccelerationStructureScratchOffsetAlignment,
				}
			),
		}};
	}

	commandRecorder.buildTLAS({
		.tlas = _tlas,
		.instance_info = tlas_instance_info,
		.scratch_buffer = tlas_scratch_buffer,
	});
}

void c3d::PathTracePass::createPipelineState()
{
	_computeShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/path tracing/trace.slang"},
		}
	);
}

void c3d::PathTracePass::createImages()
{
	_lightImage = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Light image",
			.format = SceneRenderer::ACCUMULATION_FORMAT,
			.extent = {_size, 1},
			.usages =
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eSampled,
		}
	);
}
