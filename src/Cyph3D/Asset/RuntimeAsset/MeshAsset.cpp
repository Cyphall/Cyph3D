#include "MeshAsset.h"

#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"

#include <format>

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

void MeshAsset::load_async(AssetManagerWorkerData& workerData)
{
	MeshData meshData = _manager.getAssetProcessor().readMeshData(workerData, _signature.path);
	
	Logger::info(std::format("Uploading mesh [{}]...", _signature.path));
	
	vk::BufferUsageFlags vertexBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer;
	if (Engine::getVKContext().isRayTracingSupported())
	{
		vertexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
	}
	_vertexBuffer = VKBuffer<VertexData>::create(
		Engine::getVKContext(),
		meshData.vertices.size(),
		vertexBufferUsage,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	std::copy(meshData.vertices.begin(), meshData.vertices.end(), _vertexBuffer->getHostPointer());
	
	vk::BufferUsageFlags indexBufferUsage = vk::BufferUsageFlagBits::eIndexBuffer;
	if (Engine::getVKContext().isRayTracingSupported())
	{
		indexBufferUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
	}
	_indexBuffer = VKBuffer<uint32_t>::create(
		Engine::getVKContext(),
		meshData.indices.size(),
		indexBufferUsage,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
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
		
		VKPtr<VKBuffer<std::byte>> backingBuffer = VKBuffer<std::byte>::create(
			Engine::getVKContext(),
			buildSizesInfo.accelerationStructureSize,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_accelerationStructure = VKAccelerationStructure::create(
			Engine::getVKContext(),
			vk::AccelerationStructureTypeKHR::eBottomLevel,
			buildSizesInfo.accelerationStructureSize,
			backingBuffer);
		
		VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(
			Engine::getVKContext(),
			buildSizesInfo.buildScratchSize,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		workerData.computeCommandBuffer->begin();
		workerData.computeCommandBuffer->buildBottomLevelAccelerationStructure(_accelerationStructure, scratchBuffer, buildInfo);
		workerData.computeCommandBuffer->end();
		
		Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, nullptr, nullptr);
		
		workerData.computeCommandBuffer->waitExecution();
	}

	_loaded = true;
	Logger::info(std::format("Mesh [{}] uploaded succesfully", _signature.path));
}