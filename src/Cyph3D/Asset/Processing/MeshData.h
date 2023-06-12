#pragma once

#include "Cyph3D/Rendering/VertexData.h"

#include <vector>

struct MeshData
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};