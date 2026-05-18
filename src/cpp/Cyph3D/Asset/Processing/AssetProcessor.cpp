#include "AssetProcessor.h"

c3d::ImageData c3d::AssetProcessor::readImageData(std::string_view path, ImageType type)
{
	std::string cachePath = _database.getImageCachePath(path, type);
	return _imageProcessor.readImageData(path, type, cachePath);
}

c3d::MeshData c3d::AssetProcessor::readMeshData(std::string_view path)
{
	std::string cachePath = _database.getMeshCachePath(path);
	return _meshProcessor.readMeshData(path, cachePath);
}

c3d::EquirectangularSkyboxData c3d::AssetProcessor::readEquirectangularSkyboxData(std::string_view path)
{
	std::string cachePath = _database.getEquirectangularSkyboxCachePath(path);
	return _equirectangularSkyboxProcessor.readEquirectangularSkyboxData(path, cachePath);
}