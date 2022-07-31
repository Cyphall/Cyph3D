#include "Mesh.h"

#include "Cyph3D/GLObject/GLImmutableBuffer.h"

Mesh::Mesh(std::span<const Mesh::VertexData> vertexData, std::span<const GLuint> indices)
{
	_vbo = std::make_unique<GLImmutableBuffer<Mesh::VertexData>>(vertexData.size(), GL_DYNAMIC_STORAGE_BIT);
	_vbo->setData(vertexData);

	_ibo = std::make_unique<GLImmutableBuffer<GLuint>>(indices.size(), GL_DYNAMIC_STORAGE_BIT);
	_ibo->setData(indices);
}

Mesh::~Mesh()
{}

const GLImmutableBuffer<Mesh::VertexData>& Mesh::getVBO() const
{
	return *_vbo;
}

const GLImmutableBuffer<GLuint>& Mesh::getIBO() const
{
	return *_ibo;
}