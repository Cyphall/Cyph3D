#include "MeshAsset.h"

#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"

#include <format>

MeshAsset* MeshAsset::_defaultMesh = nullptr;
MeshAsset* MeshAsset::_missingMesh = nullptr;

MeshAsset::MeshAsset(AssetManager& manager, const MeshAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&MeshAsset::load_async, this);
}

MeshAsset::~MeshAsset()
{}

const VKPtr<VKBuffer<VertexData>>& MeshAsset::getVertexBuffer() const
{
	checkLoaded();
	return _vertexBuffer;
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
	
	Logger::info(std::format("Uploading mesh [{}]...", _signature.path));
	
	vk::BufferUsageFlags vertexBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer;
	vk::DeviceAddress vertexBufferAlignment = 1;
	if (Engine::getVKContext().isRayTracingSupported())
	{
		vertexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
		vertexBufferAlignment = sizeof(float);
	}
	VKBufferInfo vertexBufferInfo(meshData.vertices.size(), vertexBufferUsage);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	vertexBufferInfo.setRequiredAlignment(vertexBufferAlignment);
	
	_vertexBuffer = VKBuffer<VertexData>::create(Engine::getVKContext(), vertexBufferInfo);
	
	std::copy(meshData.vertices.begin(), meshData.vertices.end(), _vertexBuffer->getHostPointer());
	
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
	
	_indexBuffer = VKBuffer<uint32_t>::create(Engine::getVKContext(), indexBufferInfo);
	
	std::copy(meshData.indices.begin(), meshData.indices.end(), _indexBuffer->getHostPointer());
	
	if (Engine::getVKContext().isRayTracingSupported())
	{
		VKBottomLevelAccelerationStructureBuildInfo buildInfo{
			.vertexBuffer = _vertexBuffer,
			.vertexFormat = vk::Format::eR32G32B32Sfloat,
			.vertexStride = sizeof(VertexData),
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
			temporaryBackingBuffer);
		
		VKBufferInfo scratchBufferInfo(buildSizesInfo.buildScratchSize,vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
		scratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		scratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);
		
		VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), scratchBufferInfo);
		
		VKPtr<VKAccelerationStructureCompactedSizeQuery> compactedSizeQuery = VKAccelerationStructureCompactedSizeQuery::create(Engine::getVKContext());
		
		workerData.computeCommandBuffer->begin();
		workerData.computeCommandBuffer->buildBottomLevelAccelerationStructure(temporaryAccelerationStructure, scratchBuffer, buildInfo);
		workerData.computeCommandBuffer->memoryBarrier(
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
			vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
			vk::AccessFlagBits2::eAccelerationStructureReadKHR);
		workerData.computeCommandBuffer->queryAccelerationStructureCompactedSize(temporaryAccelerationStructure, compactedSizeQuery);
		workerData.computeCommandBuffer->end();
		
		Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, nullptr, nullptr);
		
		workerData.computeCommandBuffer->waitExecution();
		
		vk::DeviceSize compactedSize = compactedSizeQuery->getCompactedSize();
		
		VKBufferInfo backingBufferInfo(compactedSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
		backingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		VKPtr<VKBuffer<std::byte>> backingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), backingBufferInfo);
		
		_accelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			compactedSize,
			backingBuffer);
		
		workerData.computeCommandBuffer->begin();
		workerData.computeCommandBuffer->compactAccelerationStructure(temporaryAccelerationStructure, _accelerationStructure);
		workerData.computeCommandBuffer->end();
		
		Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, nullptr, nullptr);
		
		workerData.computeCommandBuffer->waitExecution();
		workerData.computeCommandBuffer->reset();
	}

	_loaded = true;
	Logger::info(std::format("Mesh [{}] uploaded succesfully", _signature.path));
}