#include "ModelAsset.h"

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"

#include <format>

ModelAsset::ModelAsset(AssetManager& manager, const ModelAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading model {}", _signature.path));
	_manager.addMainThreadTask(&ModelAsset::load_step1_mt, this);
}

ModelAsset::~ModelAsset()
{}

const VKPtr<VKBuffer<VertexData>>& ModelAsset::getVertexBuffer() const
{
	checkLoaded();
	return _vertexBuffer;
}

const VKPtr<VKBuffer<uint32_t>>& ModelAsset::getIndexBuffer() const
{
	checkLoaded();
	return _indexBuffer;
}

bool ModelAsset::load_step1_mt()
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
	Logger::info(std::format("Model {} loaded", _signature.path));

	return true;
}