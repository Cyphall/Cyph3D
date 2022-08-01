#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

template<typename T>
class GLBuffer;

class Mesh
{
public:
	struct VertexData
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};
	
	Mesh(std::unique_ptr<GLBuffer<Mesh::VertexData>>&& vertexData, std::unique_ptr<GLBuffer<GLuint>>&& indices);
	~Mesh();
	
	const GLBuffer<Mesh::VertexData>& getVBO() const;
	const GLBuffer<GLuint>& getIBO() const;
	
private:
	std::unique_ptr<GLBuffer<Mesh::VertexData>> _vbo;
	std::unique_ptr<GLBuffer<GLuint>> _ibo;
};