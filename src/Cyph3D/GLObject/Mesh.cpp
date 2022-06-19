#include "Mesh.h"

Mesh::Mesh(std::span<const Mesh::VertexData> vertexData, std::span<const GLuint> indices):
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
