#include "Cyph3D/ResourceManagement/Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <format>
#include <chrono>

void Model::loadResourceImpl()
{
	std::string path = std::format("resources/meshes/{}.obj", _name);
	
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate);
	aiMesh* mesh = scene->mMeshes[0];
	
	std::vector<Mesh::VertexData> vertexData;
	vertexData.reserve(mesh->mNumVertices);
	
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		aiVector3D v = mesh->mVertices[i];
		aiVector3D uv = mesh->mTextureCoords[0][i];
		aiVector3D n = mesh->mNormals[i];
		aiVector3D t = mesh->mTangents[i];
		
		vertexData.emplace_back(
				*reinterpret_cast<glm::vec3*>(&v),
				glm::vec2(*reinterpret_cast<glm::vec3*>(&uv)),
				*reinterpret_cast<glm::vec3*>(&n),
				*reinterpret_cast<glm::vec3*>(&t));
	}
	
	std::vector<GLuint> indices;
	indices.reserve(mesh->mNumFaces * 3);
	
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	_resource = std::make_unique<Mesh>(vertexData, indices);
	
	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto timeout = std::chrono::seconds(10);
	glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
}