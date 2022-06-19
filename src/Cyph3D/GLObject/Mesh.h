#pragma once

#include "Buffer.h"
#include "VertexArray.h"
#include <glm/glm.hpp>
#include <span>

class Mesh
{
public:
	struct VertexData
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		
		VertexData(const glm::vec3& position, const glm::vec2& uv, const glm::vec3& normal, const glm::vec3& tangent):
				position(position), uv(uv), normal(normal), tangent(tangent)
		{}
	};
	
	Mesh(std::span<const Mesh::VertexData> vertexData, std::span<const GLuint> indices);
	Mesh(const Mesh& other) = delete;
	
	const Buffer<Mesh::VertexData>& getVBO() const;
	const Buffer<GLuint>& getIBO() const;
	
private:
	Buffer<Mesh::VertexData> _vbo;
	Buffer<GLuint> _ibo;
};


