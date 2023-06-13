#pragma once

#include "Cyph3D/Asset/Processing/MeshData.h"
#include "Cyph3D/Asset/AssetManagerWorkerData.h"

#include <string_view>

class MeshProcessor
{
public:
	MeshData readMeshData(AssetManagerWorkerData& workerData, std::string_view path, std::string_view cachePath);
};