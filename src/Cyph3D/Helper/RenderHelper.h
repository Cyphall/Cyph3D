#pragma once

#include <glm/glm.hpp>
#include <memory>

class GLVertexArray;
template<typename T>
class GLImmutableBuffer;

class RenderHelper
{
public:
	static void initDrawScreenQuad();
	static void drawScreenQuad();
	
private:
	struct VertexData
	{
		glm::vec2 position;
		glm::vec2 uv;
	};
	
	static std::unique_ptr<GLVertexArray> _quadVAO;
	static std::unique_ptr<GLImmutableBuffer<RenderHelper::VertexData>> _quadVBO;
};