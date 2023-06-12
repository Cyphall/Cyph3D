#pragma once

#include "Cyph3D/Asset/Processing/MeshData.h"

#include <string_view>

class MeshProcessor
{
public:
	MeshData readMeshData(std::string_view path, std::string_view cachePath);
};