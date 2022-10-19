#pragma once

#include "Cyph3D/GLObject/Mesh.h"

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct MeshData
{
	std::vector<Mesh::VertexData> vertices;
	std::vector<GLuint> indices;
};