#include "MeshAsset.h"

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

bool MeshAsset::load_step1_mt()
{
	MeshData meshData = _manager.readMeshData(_signature.path);
	
	_vertexBuffer = VKBuffer<VertexData>::create(
		Engine::getVKContext(),
		meshData.vertices.size(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	VertexData* vertexBufferPtr = _vertexBuffer->map();
	std::copy(meshData.vertices.begin(), meshData.vertices.end(), vertexBufferPtr);
	_vertexBuffer->unmap();
	
	_indexBuffer = VKBuffer<uint32_t>::create(
		Engine::getVKContext(),
		meshData.indices.size(),
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	uint32_t* indexBufferPtr = _indexBuffer->map();
	std::copy(meshData.indices.begin(), meshData.indices.end(), indexBufferPtr);
	_indexBuffer->unmap();

	_loaded = true;
	Logger::info(std::format("Mesh {} loaded", _signature.path));

	return true;
}