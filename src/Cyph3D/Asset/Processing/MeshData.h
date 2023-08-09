#pragma once

#include "Cyph3D/Rendering/VertexData.h"

#include <vector>

struct MeshData
{
	std::vector<PositionVertexData> positionVertices;
	std::vector<FullVertexData> fullVertices;
	std::vector<uint32_t> indices;
};