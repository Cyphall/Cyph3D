#include "Cyph3D/Helper/RenderHelper.h"
#include <vector>

std::unique_ptr<VertexArray> RenderHelper::_quadVAO;
std::unique_ptr<Buffer<RenderHelper::VertexData>> RenderHelper::_quadVBO;

void RenderHelper::initDrawScreenQuad()
{
	_quadVAO = std::make_unique<VertexArray>();
	
	RenderHelper::VertexData BL = {{-1, -1}, {0, 0}};
	RenderHelper::VertexData BR = {{ 3, -1}, {2, 0}};
	RenderHelper::VertexData TL = {{-1,  3}, {0, 2}};
	
	std::vector<RenderHelper::VertexData> data = {BL, BR, TL};
	
	_quadVBO = std::make_unique<Buffer<RenderHelper::VertexData>>(data.size(), GL_DYNAMIC_STORAGE_BIT);
	_quadVBO->setData(data);
	
	_quadVAO->defineFormat(0, 0, 2, GL_FLOAT, offsetof(RenderHelper::VertexData, position));
	_quadVAO->defineFormat(0, 1, 2, GL_FLOAT, offsetof(RenderHelper::VertexData, uv));
	_quadVAO->bindBufferToSlot(*_quadVBO, 0);
}

void RenderHelper::drawScreenQuad()
{
	_quadVAO->bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);
}