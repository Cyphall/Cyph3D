#pragma once

#include "Cyph3D/Rendering/VertexData.h"

#include <vector>

struct MeshData
{
	std::vector<PositionVertexData> positionVertices;
	std::vector<MaterialVertexData> materialVertices;
	std::vector<uint32_t> indices;
	glm::vec3 boundingBoxMin;
	glm::vec3 boundingBoxMax;
};