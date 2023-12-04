#include "MeshAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <format>

MeshAsset* MeshAsset::_defaultMesh = nullptr;
MeshAsset* MeshAsset::_missingMesh = nullptr;

MeshAsset::MeshAsset(AssetManager& manager, const MeshAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&MeshAsset::load_async, this);
}

MeshAsset::~MeshAsset() = default;

const VKPtr<VKBuffer<PositionVertexData>>& MeshAsset::getPositionVertexBuffer() const
{
	checkLoaded();
	return _positionVertexBuffer;
}

const VKPtr<VKBuffer<FullVertexData>>& MeshAsset::getFullVertexBuffer() const
{
	checkLoaded();
	return _fullVertexBuffer;
}

const VKPtr<VKBuffer<uint32_t>>& MeshAsset::getIndexBuffer() const
{
	checkLoaded();
	return _indexBuffer;
}

const VKPtr<VKAccelerationStructure>& MeshAsset::getAccelerationStructure() const
{
	checkLoaded();
	return _accelerationStructure;
}

const glm::vec3& MeshAsset::getBoundingBoxMin() const
{
	checkLoaded();
	return _boundingBoxMin;
}

const glm::vec3& MeshAsset::getBoundingBoxMax() const
{
	checkLoaded();
	return _boundingBoxMax;
}

void MeshAsset::initDefaultAndMissing()
{
	_defaultMesh = Engine::getAssetManager().loadMesh("meshes/internal/Default Mesh/Default Mesh.obj");
	_missingMesh = Engine::getAssetManager().loadMesh("meshes/internal/Missing Mesh/Missing Mesh.obj");
}

MeshAsset* MeshAsset::getDefaultMesh()
{
	return _defaultMesh;
}

MeshAsset* MeshAsset::getMissingMesh()
{
	return _missingMesh;
}

void MeshAsset::load_async(AssetManagerWorkerData& workerData)
{
	MeshData meshData = _manager.getAssetProcessor().readMeshData(workerData, _signature.path);

	Logger::info("Uploading mesh [{}]...", _signature.path);

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
		vk::BufferUsageFlags fullVertexBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer;
		if (Engine::getVKContext().isRayTracingSupported())
		{
			fullVertexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
		}
		VKBufferInfo fullVertexBufferInfo(meshData.fullVertices.size(), fullVertexBufferUsage);
		fullVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		fullVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		fullVertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		fullVertexBufferInfo.setName(std::format("{}.FullVertexBuffer", _signature.path));

		_fullVertexBuffer = VKBuffer<FullVertexData>::create(Engine::getVKContext(), fullVertexBufferInfo);

		std::ranges::copy(meshData.fullVertices, _fullVertexBuffer->getHostPointer());
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
		VKBottomLevelAccelerationStructureBuildInfo buildInfo{
			.vertexBuffer = _positionVertexBuffer,
			.vertexFormat = vk::Format::eR32G32B32Sfloat,
			.vertexStride = sizeof(PositionVertexData),
			.indexBuffer = _indexBuffer,
			.indexType = vk::IndexType::eUint32
		};

		vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getBottomLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);

		VKBufferInfo temporaryBackingBufferInfo(buildSizesInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
		temporaryBackingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);

		VKPtr<VKBuffer<std::byte>> temporaryBackingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), temporaryBackingBufferInfo);

		VKPtr<VKAccelerationStructure> temporaryAccelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			buildSizesInfo.accelerationStructureSize,
			temporaryBackingBuffer
		);

		VKBufferInfo scratchBufferInfo(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
		scratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		scratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);

		VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), scratchBufferInfo);

		VKPtr<VKAccelerationStructureCompactedSizeQuery> compactedSizeQuery = VKAccelerationStructureCompactedSizeQuery::create(Engine::getVKContext());

		workerData.computeCommandBuffer->begin();

		workerData.computeCommandBuffer->bufferMemoryBarrier(
			_positionVertexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		workerData.computeCommandBuffer->bufferMemoryBarrier(
			_indexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		workerData.computeCommandBuffer->buildBottomLevelAccelerationStructure(temporaryAccelerationStructure, scratchBuffer, buildInfo);

		workerData.computeCommandBuffer->releaseBufferOwnership(
			_positionVertexBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR,
			Engine::getVKContext().getMainQueue()
		);

		workerData.computeCommandBuffer->releaseBufferOwnership(
			_indexBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR,
			Engine::getVKContext().getMainQueue()
		);

		workerData.computeCommandBuffer->bufferMemoryBarrier(
			temporaryBackingBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
			vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		workerData.computeCommandBuffer->queryAccelerationStructureCompactedSize(temporaryAccelerationStructure, compactedSizeQuery);

		workerData.computeCommandBuffer->end();

		Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, {}, {});

		workerData.computeCommandBuffer->waitExecution();
		workerData.computeCommandBuffer->reset();

		vk::DeviceSize compactedSize = compactedSizeQuery->getCompactedSize();

		VKBufferInfo backingBufferInfo(compactedSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
		backingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		backingBufferInfo.setName(std::format("{}.AccelerationStructureBuffer", _signature.path));

		VKPtr<VKBuffer<std::byte>> backingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), backingBufferInfo);

		_accelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			compactedSize,
			backingBuffer
		);

		workerData.computeCommandBuffer->begin();

		workerData.computeCommandBuffer->compactAccelerationStructure(temporaryAccelerationStructure, _accelerationStructure);

		workerData.computeCommandBuffer->releaseBufferOwnership(
			backingBuffer,
			vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR,
			vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
			Engine::getVKContext().getMainQueue()
		);

		workerData.computeCommandBuffer->end();

		Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, {}, {});

		workerData.computeCommandBuffer->waitExecution();
		workerData.computeCommandBuffer->reset();

		workerData.graphicsCommandBuffer->begin();

		workerData.graphicsCommandBuffer->acquireBufferOwnership(
			_positionVertexBuffer,
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		workerData.graphicsCommandBuffer->bufferMemoryBarrier(
			_fullVertexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eVertexAttributeInput | vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eVertexAttributeRead | vk::AccessFlagBits2::eShaderStorageRead
		);

		workerData.graphicsCommandBuffer->acquireBufferOwnership(
			_indexBuffer,
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eIndexInput | vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eIndexRead | vk::AccessFlagBits2::eShaderStorageRead
		);

		workerData.graphicsCommandBuffer->acquireBufferOwnership(
			backingBuffer,
			Engine::getVKContext().getComputeQueue(),
			vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR
		);

		workerData.graphicsCommandBuffer->end();

		Engine::getVKContext().getMainQueue().submit(workerData.graphicsCommandBuffer, {}, {});

		workerData.graphicsCommandBuffer->waitExecution();
		workerData.graphicsCommandBuffer->reset();
	}
	else
	{
		workerData.graphicsCommandBuffer->begin();

		workerData.graphicsCommandBuffer->bufferMemoryBarrier(
			_positionVertexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		workerData.graphicsCommandBuffer->bufferMemoryBarrier(
			_fullVertexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eVertexAttributeInput,
			vk::AccessFlagBits2::eVertexAttributeRead
		);

		workerData.graphicsCommandBuffer->bufferMemoryBarrier(
			_indexBuffer,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eIndexInput,
			vk::AccessFlagBits2::eIndexRead
		);

		workerData.graphicsCommandBuffer->end();

		Engine::getVKContext().getMainQueue().submit(workerData.graphicsCommandBuffer, {}, {});

		workerData.graphicsCommandBuffer->waitExecution();
		workerData.graphicsCommandBuffer->reset();
	}

	_boundingBoxMin = meshData.boundingBoxMin;
	_boundingBoxMax = meshData.boundingBoxMax;

	_loaded = true;
	Logger::info("Mesh [{}] uploaded succesfully", _signature.path);

	_changed();
}