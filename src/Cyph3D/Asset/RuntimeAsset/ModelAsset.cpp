#include "ModelAsset.h"

#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <format>

ModelAsset::ModelAsset(AssetManager& manager, const ModelAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading model {}", _signature.path));
	_manager.addMainThreadTask(&ModelAsset::load_step1_mt, this);
}

ModelAsset::~ModelAsset()
{}

const Mesh& ModelAsset::getMesh() const
{
	checkLoaded();
	return *_glMesh;
}

bool ModelAsset::load_step1_mt()
{
	MeshData meshData = _manager.readMeshData(_signature.path);
	
	std::unique_ptr<GLImmutableBuffer<Mesh::VertexData>> vertexBuffer = std::make_unique<GLImmutableBuffer<Mesh::VertexData>>(meshData.vertices.size(), GL_DYNAMIC_STORAGE_BIT);
	vertexBuffer->setData(meshData.vertices);
	
	std::unique_ptr<GLImmutableBuffer<GLuint>> indexBuffer = std::make_unique<GLImmutableBuffer<GLuint>>(meshData.indices.size(), GL_DYNAMIC_STORAGE_BIT);
	indexBuffer->setData(meshData.indices);
	
	_glMesh = std::make_unique<Mesh>(std::move(vertexBuffer), std::move(indexBuffer));

	_loaded = true;
	Logger::info(std::format("Model {} loaded", _signature.path));

	return true;
}