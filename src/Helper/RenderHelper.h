#pragma once

#include <memory>
#include "../GLObject/VertexArray.h"

class RenderHelper
{
public:
	static void initDrawScreenQuad();
	static void drawScreenQuad();
	
private:
	static std::unique_ptr<VertexArray> _quadVAO;
	static std::unique_ptr<VertexBuffer<float>> _quadVBO;
};
