#include "AssetProcessor.h"

ImageData AssetProcessor::readImageData(std::string_view path, ImageType type)
{
	std::string cachePath = _database.getImageCachePath(path, type);
	return _imageProcessor.readImageData(path, type, cachePath);
}

MeshData AssetProcessor::readMeshData(std::string_view path)
{
	std::string cachePath = _database.getMeshCachePath(path);
	return _meshProcessor.readMeshData(path, cachePath);
}