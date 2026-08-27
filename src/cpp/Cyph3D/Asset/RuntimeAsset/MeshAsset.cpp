#include "MeshAsset.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Engine.h>

#include <CyphGPU/BLAS.hpp>
#include <CyphGPU/Buffer.hpp>
#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <spdlog/spdlog.h>

c3d::MeshAsset* c3d::MeshAsset::_defaultMesh = nullptr;
c3d::MeshAsset* c3d::MeshAsset::_missingMesh = nullptr;

c3d::MeshAsset::MeshAsset(AssetManager& manager, const MeshAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&MeshAsset::load_async, this);
}

c3d::MeshAsset::~MeshAsset() = default;

const cgpu::BufferPtr& c3d::MeshAsset::getPositionVertexBuffer() const
{
	checkLoaded();
	return _positionVertexBuffer;
}

const cgpu::BufferPtr& c3d::MeshAsset::getMaterialVertexBuffer() const
{
	checkLoaded();
	return _materialVertexBuffer;
}

const cgpu::BufferPtr& c3d::MeshAsset::getIndexBuffer() const
{
	checkLoaded();
	return _indexBuffer;
}

uint32_t c3d::MeshAsset::getIndexCount() const
{
	checkLoaded();
	return _indexBuffer->getDesc().size / sizeof(uint32_t);
}

vk::IndexType c3d::MeshAsset::getIndexType() const
{
	return vk::IndexType::eUint32;
}

const cgpu::BLASPtr& c3d::MeshAsset::getBLAS() const
{
	checkLoaded();
	return _blas;
}

const glm::vec3& c3d::MeshAsset::getBoundingBoxMin() const
{
	checkLoaded();
	return _boundingBoxMin;
}

const glm::vec3& c3d::MeshAsset::getBoundingBoxMax() const
{
	checkLoaded();
	return _boundingBoxMax;
}

void c3d::MeshAsset::initDefaultAndMissing()
{
	_defaultMesh = Engine::getAssetManager().loadMesh("meshes/internal/Default Mesh/Default Mesh.obj");
	_missingMesh = Engine::getAssetManager().loadMesh("meshes/internal/Missing Mesh/Missing Mesh.obj");
}

c3d::MeshAsset* c3d::MeshAsset::getDefaultMesh()
{
	return _defaultMesh;
}

c3d::MeshAsset* c3d::MeshAsset::getMissingMesh()
{
	return _missingMesh;
}

void c3d::MeshAsset::load_async()
{
	spdlog::info("Loading mesh [{}]...", _signature.path);

	MeshData meshData = _manager.getAssetProcessor().readMeshData(_signature.path);

	bool hasRayTracing = static_cast<bool>(Engine::getDeviceSession()->getDevice()->getCapabilities() & cgpu::Device::Capability::eRayTracing);

	{
		vk::BufferUsageFlags2 usages;
		if (hasRayTracing)
		{
			usages |= vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR;
		}

		_positionVertexBuffer = cgpu::Buffer::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("{}.PositionVertexBuffer", _signature.path),
				.size = meshData.positionVertices.size() * sizeof(PositionVertexData),
				.usages = usages,
				.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
				.min_alignment = alignof(PositionVertexData),
			}
		);

		std::ranges::copy(meshData.positionVertices, _positionVertexBuffer->getHostPtr<PositionVertexData>());
	}

	{
		_materialVertexBuffer = cgpu::Buffer::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("{}.MaterialVertexData", _signature.path),
				.size = meshData.materialVertices.size() * sizeof(MaterialVertexData),
				.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
				.min_alignment = alignof(MaterialVertexData),
			}
		);

		std::ranges::copy(meshData.materialVertices, _materialVertexBuffer->getHostPtr<MaterialVertexData>());
	}

	{
		vk::BufferUsageFlags2 usages = vk::BufferUsageFlagBits2::eIndexBuffer;
		if (hasRayTracing)
		{
			usages |= vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR;
		}

		_indexBuffer = cgpu::Buffer::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("{}.IndexBuffer", _signature.path),
				.size = meshData.indices.size() * sizeof(uint32_t),
				.usages = usages,
				.memory_type = cgpu::MemoryType::eCPUVisibleGPU,
				.min_alignment = alignof(uint32_t),
			}
		);

		std::ranges::copy(meshData.indices, _indexBuffer->getHostPtr<uint32_t>());
	}

	if (hasRayTracing)
	{
		cgpu::BLAS::ASInfo blasInfo{
			.vertex_buffer = {
				.count = static_cast<uint32_t>(meshData.positionVertices.size()),
				.format = vk::Format::eR32G32B32Sfloat,
				.stride = sizeof(PositionVertexData),
			},
			.index_buffer = {{
				.count = static_cast<uint32_t>(meshData.indices.size()),
				.type = vk::IndexType::eUint32,
			}},
			.opaque = true,
		};

		auto sizes = cgpu::BLAS::calcSizes(Engine::getDeviceSession(), blasInfo);

		cgpu::BufferPtr blas_buffer = cgpu::Buffer::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("{}.BLASBuffer", _signature.path),
				.size = sizes.accelerationStructureSize,
				.usages = vk::BufferUsageFlagBits2::eAccelerationStructureStorageKHR,
				.min_alignment = 256,
			}
		);

		_blas = cgpu::BLAS::create(
			Engine::getDeviceSession(),
			{
				.name = std::format("{}.BLAS", _signature.path),
				.as_info = blasInfo,
				.buffer = blas_buffer,
				.sizes = sizes,
			}
		);

		std::optional<cgpu::CommandRecorder::BLASParams::ScratchBuffer> blas_scratch_buffer;
		if (_blas->getDesc().sizes.buildScratchSize > 0)
		{
			blas_scratch_buffer = {{
				.buffer = cgpu::Buffer::create(
					Engine::getDeviceSession(),
					{
						.name = "BLAS (build scratch memory)",
						.size = _blas->getDesc().sizes.buildScratchSize,
						.usages = vk::BufferUsageFlagBits2::eStorageBuffer,
						.min_alignment = Engine::getDeviceSession()->getDevice()->getProperties<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>().minAccelerationStructureScratchOffsetAlignment,
					}
				),
			}};
		}

		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncComputeQueue());

		commandRecorder.buildBLAS({
			.blas = _blas,
			.vertex_buffer = {{
				.buffer = _positionVertexBuffer,
			}},
			.index_buffer = {{
				.buffer = _indexBuffer,
			}},
			.scratch_buffer = blas_scratch_buffer,
		});

		commandRecorder.submit().waitFinished();

		assetCommandContext->finish();
	}

	_boundingBoxMin = meshData.boundingBoxMin;
	_boundingBoxMax = meshData.boundingBoxMax;

	_loaded = true;
	spdlog::info("Mesh [{}] uploaded succesfully", _signature.path);

	_changed();
}
