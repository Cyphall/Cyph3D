#pragma once

#include "Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.h"
#include "Cyph3D/Asset/Processing/ImageProcessor.h"
#include "Cyph3D/Asset/Processing/MeshProcessor.h"

struct AssetManagerWorkerData;

class AssetProcessor
{
public:
	ImageData readImageData(AssetManagerWorkerData& workerData, std::string_view path, ImageType type);
	MeshData readMeshData(AssetManagerWorkerData& workerData, std::string_view path);
	
private:
	AssetProcessingCacheDatabase _database;
	
	ImageProcessor _imageProcessor;
	MeshProcessor _meshProcessor;
};