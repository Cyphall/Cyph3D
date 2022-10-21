#pragma once

#include "Cyph3D/Asset/Processor/MeshData.h"

#include <string_view>

class MeshProcessor
{
private:
	friend class AssetManager;

	static MeshData readMeshData(std::string_view path, std::string_view cachePath);
};