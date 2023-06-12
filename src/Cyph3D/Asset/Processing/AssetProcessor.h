#pragma once

#include "Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.h"
#include "Cyph3D/Asset/Processing/ImageProcessor.h"
#include "Cyph3D/Asset/Processing/MeshProcessor.h"

class AssetProcessor
{
public:
	ImageData readImageData(std::string_view path, ImageType type);
	MeshData readMeshData(std::string_view path);
	
private:
	AssetProcessingCacheDatabase _database;
	
	ImageProcessor _imageProcessor;
	MeshProcessor _meshProcessor;
};