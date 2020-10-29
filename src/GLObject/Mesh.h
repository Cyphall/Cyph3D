#pragma once

#include "Buffer.h"
#include "VertexBuffer.h"
#include "../ResourceManagement/VertexData.h"
#include "VertexArray.h"

class Mesh
{
public:
	Mesh(const std::vector<VertexData>& vertexData, const std::vector<int>& indices);
	Mesh(const Mesh& other) = delete;
	
	void render();
private:
	VertexBuffer<VertexData> _vbo;
	Buffer<int> _ibo;
	
	VertexArray _vao;
};


