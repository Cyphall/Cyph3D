#include "Mesh.h"

#include "Cyph3D/GLObject/GLImmutableBuffer.h"

Mesh::Mesh(std::unique_ptr<GLBuffer<Mesh::VertexData>>&& vertexData, std::unique_ptr<GLBuffer<GLuint>>&& indices)
{
	_vbo = std::forward<std::unique_ptr<GLBuffer<Mesh::VertexData>>>(vertexData);
	_ibo = std::forward<std::unique_ptr<GLBuffer<GLuint>>>(indices);
}

Mesh::~Mesh()
{}

const GLBuffer<Mesh::VertexData>& Mesh::getVBO() const
{
	return *_vbo;
}

const GLBuffer<GLuint>& Mesh::getIBO() const
{
	return *_ibo;
}