#include "Model.h"

#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/Logging/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <chrono>
#include <format>

struct Model::LoadData
{
	ResourceManager* rm;

	// step 1: load mesh into vectors
	std::vector<Mesh::VertexData> vertices;
	std::vector<GLuint> indices;

	// step 2: create buffers and fill them
	std::unique_ptr<GLImmutableBuffer<Mesh::VertexData>> vertexBuffer;
	std::unique_ptr<GLImmutableBuffer<GLuint>> indexBuffer;
	GLsync fence;

	// step 3: wait for the fence
};

Model::Model(const std::string& name, ResourceManager& rm):
	Resource(name)
{
	_loadData = std::make_unique<LoadData>();
	_loadData->rm = &rm;

	Logger::info(std::format("Loading model \"{}\"", getName()));
	_loadData->rm->addThreadPoolTask(std::bind(&Model::load_step1_tp, this));
}

Model::~Model()
{}

void Model::load_step1_tp()
{
	std::string path = std::format("resources/meshes/{}.obj", _name);

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate);
	aiMesh* mesh = scene->mMeshes[0];

	_loadData->vertices.resize(mesh->mNumVertices);

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		std::memcpy(&_loadData->vertices[i].position, &mesh->mVertices[i], 3 * sizeof(float));
		std::memcpy(&_loadData->vertices[i].uv, &mesh->mTextureCoords[0][i], 2 * sizeof(float));
		std::memcpy(&_loadData->vertices[i].normal, &mesh->mNormals[i], 3 * sizeof(float));
		std::memcpy(&_loadData->vertices[i].tangent, &mesh->mTangents[i], 3 * sizeof(float));
	}

	_loadData->indices.resize(mesh->mNumFaces * 3);

	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		std::memcpy(&_loadData->indices[i*3], mesh->mFaces[i].mIndices, 3 * sizeof(GLuint));
	}
	
	_loadData->rm->addMainThreadTask(std::bind(&Model::load_step2_mt, this));
}

bool Model::load_step2_mt()
{
	_loadData->vertexBuffer = std::make_unique<GLImmutableBuffer<Mesh::VertexData>>(_loadData->vertices.size(), GL_DYNAMIC_STORAGE_BIT);
	_loadData->vertexBuffer->setData(_loadData->vertices);
	_loadData->vertices = std::vector<Mesh::VertexData>(); // clear & free
	
	_loadData->indexBuffer = std::make_unique<GLImmutableBuffer<GLuint>>(_loadData->indices.size(), GL_DYNAMIC_STORAGE_BIT);
	_loadData->indexBuffer->setData(_loadData->indices);
	_loadData->indices = std::vector<GLuint>(); // clear & free

	_loadData->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	_loadData->rm->addMainThreadTask(std::bind(&Model::load_step3_mt, this));
	return true;
}

bool Model::load_step3_mt()
{
	if (glClientWaitSync(_loadData->fence, 0, 0) != GL_ALREADY_SIGNALED)
	{
		return false;
	}
	
	_resource = std::make_unique<Mesh>(std::move(_loadData->vertexBuffer), std::move(_loadData->indexBuffer));
	_loadData.reset();
	_ready = true;
	Logger::info(std::format("Model \"{}\" loaded", getName()));
	return true;
}