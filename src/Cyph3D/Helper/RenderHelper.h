#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Cyph3D/GLObject/VertexArray.h"

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
	
	static std::unique_ptr<VertexArray> _quadVAO;
	static std::unique_ptr<Buffer<RenderHelper::VertexData>> _quadVBO;
};