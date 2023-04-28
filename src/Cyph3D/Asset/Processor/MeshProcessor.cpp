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

	uint8_t version = 1;
	FileHelper::write(file, &version);

	FileHelper::write(file, meshData.vertices);

	FileHelper::write(file, meshData.indices);
}

static bool readProcessedMesh(const std::filesystem::path& path, MeshData& meshData)
{
	std::ifstream file = FileHelper::openFileForReading(path);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version > 1)
	{
		return false;
	}

	FileHelper::read(file, meshData.vertices);

	FileHelper::read(file, meshData.indices);

	return true;
}

static MeshData processMesh(const std::filesystem::path& input, const std::filesystem::path& output)
{
	MeshData meshData;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(input.generic_string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
	aiMesh* mesh = scene->mMeshes[0];

	meshData.vertices.resize(mesh->mNumVertices);

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		std::memcpy(&meshData.vertices[i].position, &mesh->mVertices[i], 3 * sizeof(float));
		std::memcpy(&meshData.vertices[i].uv, &mesh->mTextureCoords[0][i], 2 * sizeof(float));
		std::memcpy(&meshData.vertices[i].normal, &mesh->mNormals[i], 3 * sizeof(float));
		std::memcpy(&meshData.vertices[i].tangent, &mesh->mTangents[i], 3 * sizeof(float));
	}

	meshData.indices.resize(mesh->mNumFaces * 3);

	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		std::memcpy(&meshData.indices[i*3], mesh->mFaces[i].mIndices, 3 * sizeof(uint32_t));
	}

	writeProcessedMesh(output, meshData);

	return meshData;
}

MeshData MeshProcessor::readMeshData(std::string_view path, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	MeshData meshData;

	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info(std::format("Loading mesh {} from cache", path));
		if (!readProcessedMesh(cacheAbsolutePath, meshData))
		{
			Logger::warning(std::format("Cannot parse cached mesh {}. Reprocessing...", path));
			std::filesystem::remove(cacheAbsolutePath);
			meshData = processMesh(absolutePath, cacheAbsolutePath);
			Logger::info(std::format("Mesh {} reprocessed succesfully", path));
		}
	}
	else
	{
		Logger::info(std::format("Processing mesh {}", path));
		meshData = processMesh(absolutePath, cacheAbsolutePath);
		Logger::info(std::format("Mesh {} processed succesfully", path));
	}

	return meshData;
}