#include "Mesh.h"

Mesh::Mesh(const std::vector<Mesh::VertexData>& vertexData, const std::vector<GLuint>& indices):
_vbo(vertexData.size(), GL_DYNAMIC_STORAGE_BIT), _ibo(indices.size(), GL_DYNAMIC_STORAGE_BIT)
{
	_vbo.setData(vertexData);
	_ibo.setData(indices);
}

const Buffer<Mesh::VertexData>& Mesh::getVBO() const
{
	return _vbo;
}

const Buffer<GLuint>& Mesh::getIBO() const
{
	return _ibo;
}
