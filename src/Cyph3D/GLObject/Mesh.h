#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <span>

template<typename T>
class GLImmutableBuffer;

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
	~Mesh();
	
	const GLImmutableBuffer<Mesh::VertexData>& getVBO() const;
	const GLImmutableBuffer<GLuint>& getIBO() const;
	
private:
	std::unique_ptr<GLImmutableBuffer<Mesh::VertexData>> _vbo;
	std::unique_ptr<GLImmutableBuffer<GLuint>> _ibo;
};