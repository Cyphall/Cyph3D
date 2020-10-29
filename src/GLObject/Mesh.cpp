#include "Mesh.h"

Mesh::Mesh(const std::vector<VertexData>& vertexData, const std::vector<int>& indices):
_vbo(vertexData.size(), GL_DYNAMIC_STORAGE_BIT, 1), _ibo(indices.size(), GL_DYNAMIC_STORAGE_BIT)
{
	_vbo.setData(vertexData);
	_ibo.setData(indices);
	
	_vao.registerAttrib(_vbo, 0, 3, GL_FLOAT, offsetof(VertexData, position));
	_vao.registerAttrib(_vbo, 1, 2, GL_FLOAT, offsetof(VertexData, uv));
	_vao.registerAttrib(_vbo, 2, 3, GL_FLOAT, offsetof(VertexData, normal));
	_vao.registerAttrib(_vbo, 3, 3, GL_FLOAT, offsetof(VertexData, tangent));
	_vao.registerIndexBuffer(_ibo);
}

void Mesh::render()
{
	_vao.bind();
	glDrawElements(GL_TRIANGLES, _ibo.getCount(), GL_UNSIGNED_INT, nullptr);
}
