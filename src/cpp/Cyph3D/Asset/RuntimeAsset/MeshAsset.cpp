#include "MeshAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <spdlog/spdlog.h>

c3d::MeshAsset* c3d::MeshAsset::_defaultMesh = nullptr;
c3d::MeshAsset* c3d::MeshAsset::_missingMesh = nullptr;

c3d::MeshAsset::MeshAsset(AssetManager& manager, const MeshAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&MeshAsset::load_async, this);
}

c3d::MeshAsset::~MeshAsset() = default;

const std::shared_ptr<c3d::VKBuffer<c3d::PositionVertexData>>& c3d::MeshAsset::getPositionVertexBuffer() const
{
	checkLoaded();
	return _positionVertexBuffer;
}

const std::shared_ptr<c3d::VKBuffer<c3d::MaterialVertexData>>& c3d::MeshAsset::getMaterialVertexBuffer() const
{
	checkLoaded();
	return _materialVertexBuffer;
}

const std::shared_ptr<c3d::VKBuffer<uint32_t>>& c3d::MeshAsset::getIndexBuffer() const
{
	checkLoaded();
	return _indexBuffer;
}

const std::shared_ptr<c3d::VKAccelerationStructure>& c3d::MeshAsset::getAccelerationStructure() const
{
	checkLoaded();
	return _accelerationStructure;
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
	MeshData meshData = _manager.getAssetProcessor().readMeshData(_signature.path);

	spdlog::info("Uploading mesh [{}]...", _signature.path);

	{
		vk::BufferUsageFlags positionVertexBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer;
		vk::DeviceAddress positionVertexBufferAlignment = 1;
		if (Engine::getVKContext().isRayTracingSupported())
		{
			positionVertexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
			positionVertexBufferAlignment = sizeof(float);
		}
		VKBufferInfo positionVertexBufferInfo(meshData.positionVertices.size(), positionVertexBufferUsage);
		positionVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		positionVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		positionVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		positionVertexBufferInfo.setRequiredAlignment(positionVertexBufferAlignment);
		positionVertexBufferInfo.setName(std::format("{}.PositionVertexBuffer", _signature.path));

		_positionVertexBuffer = VKBuffer<PositionVertexData>::create(Engine::getVKContext(), positionVertexBufferInfo);

		std::ranges::copy(meshData.positionVertices, _positionVertexBuffer->getHostPointer());
	}

	{
		vk::BufferUsageFlags materialVertexBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer;
		if (Engine::getVKContext().isRayTracingSupported())
		{
			materialVertexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
		}
		VKBufferInfo materialVertexBufferInfo(meshData.materialVertices.size(), materialVertexBufferUsage);
		materialVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		materialVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		materialVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		materialVertexBufferInfo.setName(std::format("{}.MaterialVertexBuffer", _signature.path));

		_materialVertexBuffer = VKBuffer<MaterialVertexData>::create(Engine::getVKContext(), materialVertexBufferInfo);

		std::ranges::copy(meshData.materialVertices, _materialVertexBuffer->getHostPointer());
	}

	{
		vk::BufferUsageFlags indexBufferUsage = vk::BufferUsageFlagBits::eIndexBuffer;
		vk::DeviceAddress indexBufferAlignment = 1;
		if (Engine::getVKContext().isRayTracingSupported())
		{
			indexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
			indexBufferAlignment = sizeof(uint32_t);
		}
		VKBufferInfo indexBufferInfo(meshData.indices.size(), indexBufferUsage);
		indexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		indexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		indexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		indexBufferInfo.setRequiredAlignment(indexBufferAlignment);
		indexBufferInfo.setName(std::format("{}.IndexBuffer", _signature.path));

		_indexBuffer = VKBuffer<uint32_t>::create(Engine::getVKContext(), indexBufferInfo);

		std::ranges::copy(meshData.indices, _indexBuffer->getHostPointer());
	}

	if (Engine::getVKContext().isRayTracingSupported())
	{
		// Create temporary acceleration structure

		VKBottomLevelAccelerationStructureBuildInfo buildInfo{
			.vertexBuffer = _positionVertexBuffer,
			.vertexFormat = vk::Format::eR32G32B32Sfloat,
			.vertexStride = sizeof(PositionVertexData),
			.indexBuffer = _indexBuffer,
			.indexType = vk::IndexType::eUint32
		};

		vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getBottomLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);

		std::shared_ptr<VKAccelerationStructure> temporaryAccelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			buildSizesInfo.accelerationStructureSize
		);

		// Create scratch buffer

		VKBufferInfo scratchBufferInfo(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
		scratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		scratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

		std::shared_ptr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), scratchBufferInfo);

		// Build temporary acceleration structure and query compact size

		assetComputeCommandBuffer->begin();

		assetComputeCommandBuffer->bufferMemoryBarrier(
			temporaryAccelerationStructure->getBackingBuffer(),
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureWriteKHR
		);

		assetComputeCommandBuffer->bufferMemoryBarrier(
			scratchBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR | vk::AccessFlagBits2::eAccelerationStructureWriteKHR
		);

		assetComputeCommandBuffer->bufferMemoryBarrier(
			_positionVertexBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		assetComputeCommandBuffer->bufferMemoryBarrier(
			_indexBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		assetComputeCommandBuffer->buildBottomLevelAccelerationStructure(temporaryAccelerationStructure, scratchBuffer, buildInfo);

		assetComputeCommandBuffer->bufferMemoryBarrier(
			temporaryAccelerationStructure->getBackingBuffer(),
			vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		assetComputeCommandBuffer->releaseBufferOwnership(
			_positionVertexBuffer,
			Engine::getVKContext().getMainQueue()
		);

		assetComputeCommandBuffer->releaseBufferOwnership(
			_indexBuffer,
			Engine::getVKContext().getMainQueue()
		);

		std::shared_ptr<VKAccelerationStructureCompactedSizeQuery> compactedSizeQuery = VKAccelerationStructureCompactedSizeQuery::create(Engine::getVKContext());
		assetComputeCommandBuffer->queryAccelerationStructureCompactedSize(temporaryAccelerationStructure, compactedSizeQuery);

		assetComputeCommandBuffer->end();

		Engine::getVKContext().getComputeQueue().submit(assetComputeCommandBuffer, {}, {});

		assetComputeCommandBuffer->waitExecution();
		assetComputeCommandBuffer->reset();

		// Create temporary acceleration structure

		vk::DeviceSize compactedSize = compactedSizeQuery->getCompactedSize();

		_accelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			compactedSize
		);

		// Compact acceleration structure

		assetComputeCommandBuffer->begin();

		assetComputeCommandBuffer->bufferMemoryBarrier(
			_accelerationStructure->getBackingBuffer(),
			vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR,
			vk::AccessFlagBits2::eAccelerationStructureWriteKHR
		);

		assetComputeCommandBuffer->compactAccelerationStructure(temporaryAccelerationStructure, _accelerationStructure);

		assetComputeCommandBuffer->releaseBufferOwnership(
			_accelerationStructure->getBackingBuffer(),
			Engine::getVKContext().getMainQueue()
		);

		assetComputeCommandBuffer->end();

		Engine::getVKContext().getComputeQueue().submit(assetComputeCommandBuffer, {}, {});

		assetComputeCommandBuffer->waitExecution();
		assetComputeCommandBuffer->reset();

		assetGraphicsCommandBuffer->begin();

		assetGraphicsCommandBuffer->acquireBufferOwnership(
			_positionVertexBuffer,
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		assetGraphicsCommandBuffer->acquireBufferOwnership(
			_indexBuffer,
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eIndexInput | vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eIndexRead | vk::AccessFlagBits2::eShaderStorageRead
		);

		assetGraphicsCommandBuffer->acquireBufferOwnership(
			_accelerationStructure->getBackingBuffer(),
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		assetGraphicsCommandBuffer->bufferMemoryBarrier(
			_materialVertexBuffer,
			vk::PipelineStageFlagBits2::eVertexAttributeInput | vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eVertexAttributeRead | vk::AccessFlagBits2::eShaderStorageRead
		);

		assetGraphicsCommandBuffer->end();

		Engine::getVKContext().getMainQueue().submit(assetGraphicsCommandBuffer, {}, {});

		assetGraphicsCommandBuffer->waitExecution();
		assetGraphicsCommandBuffer->reset();
	}
	else
	{
		assetGraphicsCommandBuffer->begin();

		assetGraphicsCommandBuffer->bufferMemoryBarrier(
			_positionVertexBuffer,
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		assetGraphicsCommandBuffer->bufferMemoryBarrier(
			_materialVertexBuffer,
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		assetGraphicsCommandBuffer->bufferMemoryBarrier(
			_indexBuffer,
			vk::PipelineStageFlagBits2::eIndexInput,
			vk::AccessFlagBits2::eIndexRead
		);

		assetGraphicsCommandBuffer->end();

		Engine::getVKContext().getMainQueue().submit(assetGraphicsCommandBuffer, {}, {});

		assetGraphicsCommandBuffer->waitExecution();
		assetGraphicsCommandBuffer->reset();
	}

	_boundingBoxMin = meshData.boundingBoxMin;
	_boundingBoxMax = meshData.boundingBoxMax;

	_loaded = true;
	spdlog::info("Mesh [{}] uploaded succesfully", _signature.path);

	_changed();
}