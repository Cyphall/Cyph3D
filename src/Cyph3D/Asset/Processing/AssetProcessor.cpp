#include "AssetProcessor.h"

ImageData AssetProcessor::readImageData(AssetManagerWorkerData& workerData, std::string_view path, ImageType type)
{
	std::string cachePath = _database.getImageCachePath(path, type);
	return _imageProcessor.readImageData(workerData, path, type, cachePath);
}

MeshData AssetProcessor::readMeshData(AssetManagerWorkerData& workerData, std::string_view path)
{
	std::string cachePath = _database.getMeshCachePath(path);
	return _meshProcessor.readMeshData(workerData, path, cachePath);
}