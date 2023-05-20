#include "MeshAsset.h"

#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"

#include <format>

MeshAsset::MeshAsset(AssetManager& manager, const MeshAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading mesh {}", _signature.path));
	_manager.addMainThreadTask(&MeshAsset::load_step1_mt, this);
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

bool MeshAsset::load_step1_mt()
{
	MeshData meshData = _manager.readMeshData(_signature.path);
	
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
	
	VertexData* vertexBufferPtr = _vertexBuffer->map();
	std::copy(meshData.vertices.begin(), meshData.vertices.end(), vertexBufferPtr);
	_vertexBuffer->unmap();
	
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
	
	uint32_t* indexBufferPtr = _indexBuffer->map();
	std::copy(meshData.indices.begin(), meshData.indices.end(), indexBufferPtr);
	_indexBuffer->unmap();
	
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
		
		Engine::getVKContext().executeImmediate(
			[&](const VKPtr<VKCommandBuffer>& commandBuffer)
			{
				VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(
					Engine::getVKContext(),
					buildSizesInfo.buildScratchSize,
					vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
					vk::MemoryPropertyFlagBits::eDeviceLocal);
				
				commandBuffer->buildBottomLevelAccelerationStructure(_accelerationStructure, scratchBuffer, buildInfo);
			});
	}

	_loaded = true;
	Logger::info(std::format("Mesh {} loaded", _signature.path));

	return true;
}