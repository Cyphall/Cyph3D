#include "MeshProcessor.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Logging/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static void writeProcessedMesh(const std::filesystem::path& path, const MeshData& meshData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = FileHelper::openFileForWriting(path);

	uint8_t version = 5;
	FileHelper::write(file, &version);

	FileHelper::write(file, meshData.positionVertices);
	FileHelper::write(file, meshData.materialVertices);

	FileHelper::write(file, meshData.indices);

	FileHelper::write(file, &meshData.boundingBoxMin);
	FileHelper::write(file, &meshData.boundingBoxMax);
}

static bool readProcessedMesh(const std::filesystem::path& path, MeshData& meshData)
{
	std::ifstream file = FileHelper::openFileForReading(path);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version != 5)
	{
		return false;
	}

	FileHelper::read(file, meshData.positionVertices);
	FileHelper::read(file, meshData.materialVertices);

	FileHelper::read(file, meshData.indices);

	FileHelper::read(file, &meshData.boundingBoxMin);
	FileHelper::read(file, &meshData.boundingBoxMax);

	return true;
}

static MeshData processMesh(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output)
{
	MeshData meshData;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(input.generic_string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);

	if (scene == nullptr)
	{
		throw std::runtime_error(std::format("Unable to load mesh {} from disk", input.generic_string()));
	}

	aiMesh* mesh = scene->mMeshes[0];

	meshData.positionVertices.resize(mesh->mNumVertices);
	meshData.materialVertices.resize(mesh->mNumVertices);
	meshData.boundingBoxMin = glm::vec3(std::numeric_limits<float>::max());
	meshData.boundingBoxMax = glm::vec3(std::numeric_limits<float>::lowest());

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		glm::vec3 position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
		glm::vec2 uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
		glm::vec3 normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
		glm::vec3 tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
		glm::vec3 bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
		glm::vec4 tangentWithSign = {tangent, glm::sign(glm::dot(glm::cross(normal, tangent), bitangent))};

		meshData.positionVertices[i].position = position;
		meshData.materialVertices[i].uv = uv;
		meshData.materialVertices[i].normal = normal;
		meshData.materialVertices[i].tangent = tangentWithSign;

		meshData.boundingBoxMin = glm::min(meshData.boundingBoxMin, meshData.positionVertices[i].position);
		meshData.boundingBoxMax = glm::max(meshData.boundingBoxMax, meshData.positionVertices[i].position);
	}

	meshData.indices.resize(mesh->mNumFaces * 3);

	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		meshData.indices[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
		meshData.indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		meshData.indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	writeProcessedMesh(output, meshData);

	return meshData;
}

MeshData MeshProcessor::readMeshData(AssetManagerWorkerData& workerData, std::string_view path, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	MeshData meshData;

	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info("Loading mesh [{}] from cache...", path);
		if (readProcessedMesh(cacheAbsolutePath, meshData))
		{
			Logger::info("Mesh [{}] loaded from cache succesfully", path);
		}
		else
		{
			Logger::warning("Could not load mesh [{}] from cache. Reprocessing...", path);
			std::filesystem::remove(cacheAbsolutePath);
			meshData = processMesh(workerData, absolutePath, cacheAbsolutePath);
			Logger::info("Mesh [{}] reprocessed succesfully", path);
		}
	}
	else
	{
		Logger::info("Processing mesh [{}]", path);
		meshData = processMesh(workerData, absolutePath, cacheAbsolutePath);
		Logger::info("Mesh [{}] processed succesfully", path);
	}

	return meshData;
}