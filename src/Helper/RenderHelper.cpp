#include "RenderHelper.h"

std::unique_ptr<VertexArray> RenderHelper::_quadVAO;
std::unique_ptr<VertexBuffer<float>> RenderHelper::_quadVBO;

void RenderHelper::initDrawScreenQuad()
{
	_quadVAO = std::make_unique<VertexArray>();
	
	std::vector<float> _quadVBOData = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f,  1.0f,
		-1.0f, -1.0f,  0.0f,  0.0f,
		1.0f, -1.0f,  1.0f,  0.0f,
		
		-1.0f,  1.0f,  0.0f,  1.0f,
		1.0f, -1.0f,  1.0f,  0.0f,
		1.0f,  1.0f,  1.0f,  1.0f
	};
	
	_quadVBO = std::make_unique<VertexBuffer<float>>(_quadVBOData.size(), GL_DYNAMIC_STORAGE_BIT, 4);
	_quadVBO->setData(_quadVBOData);
	
	_quadVAO->registerAttrib(*_quadVBO, 0, 2, GL_FLOAT, 0);
	_quadVAO->registerAttrib(*_quadVBO, 1, 2, GL_FLOAT, 2 * sizeof(float));
}

void RenderHelper::drawScreenQuad()
{
	_quadVAO->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
