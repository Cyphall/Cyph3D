#pragma once

#include "Cyph3D/Asset/Processing/MeshData.h"

class MeshProcessor
{
public:
	MeshData readMeshData(std::string_view path, std::string_view cachePath);
};