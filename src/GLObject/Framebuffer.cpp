#include "Framebuffer.h"
#include <stdexcept>
#include <vector>
#include "../Helper/MathHelper.h"
#include "../Engine.h"
#include "../ResourceManagement/ResourceManager.h"
#include <fmt/core.h>
#include "../GLStateManager.h"
#include "../Helper/RenderHelper.h"

Framebuffer::Framebuffer(glm::ivec2 size):
_size(size)
{
	glCreateFramebuffers(1, &_handle);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &_handle);
}

void Framebuffer::clearAll()
{
	clearColor();
	clearDepth();
	clearStencil();
}

void Framebuffer::clearColor()
{
	float clearColor[] = {0, 0, 0, 0};
	
	for (int i = 0; i < _drawBuffers.size(); ++i)
	{
		glClearNamedFramebufferfv(_handle, GL_COLOR, i, clearColor);
	}
}

void Framebuffer::clearDepth()
{
	float clearValue = 1;
	glClearNamedFramebufferfv(_handle, GL_DEPTH, 0, &clearValue);
}

void Framebuffer::clearStencil()
{
	int clearValue = 0;
	glClearNamedFramebufferiv(_handle, GL_STENCIL, 0, &clearValue);
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _handle);
}

glm::ivec2 Framebuffer::getSize()
{
	return _size;
}

void Framebuffer::checkState()
{
	std::vector<GLenum> drawBuffers;
	drawBuffers.reserve(_drawBuffers.size());
	
	for (GLenum drawBuffer : _drawBuffers)
	{
		drawBuffers.push_back(drawBuffer);
	}
	
	//TODO: fix mapping between GL_COLOR_ATTACHMENT and GLSL layouts
	
	glNamedFramebufferDrawBuffers(_handle, drawBuffers.size(), drawBuffers.data());
	
	GLenum state = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(fmt::format("Error while creating framebuffer: {}", state));
	}
}

bool Framebuffer::isDrawBuffer(GLenum attachment)
{
	return between(attachment, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT31);
}

#pragma region Attachment setters

void Framebuffer::attach(GLenum attachment, const Texture& texture)
{
	if (texture.getSize() != _size)
	{
		throw std::runtime_error("The texture size does not match the framebuffer size");
	}
	
	glNamedFramebufferTexture(_handle, attachment, texture.getHandle(), 0);
	
	if (isDrawBuffer(attachment))
	{
		_drawBuffers.insert(attachment);
	}
	
	checkState();
}

void Framebuffer::attach(GLenum attachment, const Cubemap& cubemap)
{
	if (cubemap.getSize() != _size)
	{
		throw std::runtime_error("The cubemap size does not match the framebuffer size");
	}
	
	glNamedFramebufferTexture(_handle, attachment, cubemap.getHandle(), 0);
	
	if (isDrawBuffer(attachment))
	{
		_drawBuffers.insert(attachment);
	}
	
	checkState();
}

void Framebuffer::attach(GLenum attachment, const Cubemap& cubemap, int face)
{
	if (cubemap.getSize() != _size)
	{
		throw std::runtime_error("The cubemap size does not match the framebuffer size");
	}
	
	if (!between(face, 0, 6))
	{
		throw std::runtime_error("The cubemap size does not match the framebuffer size");
	}
	
	glNamedFramebufferTextureLayer(_handle, attachment, cubemap.getHandle(), 0, face);
	
	if (isDrawBuffer(attachment))
	{
		_drawBuffers.insert(attachment);
	}
	
	checkState();
}

void Framebuffer::detach(GLenum attachment)
{
	glNamedFramebufferTexture(_handle, attachment, 0, 0);
	
	if (isDrawBuffer(attachment))
	{
		_drawBuffers.erase(attachment);
	}
}

#pragma endregion Attachment setters

#pragma region DrawToDefault

ShaderProgram* Framebuffer::_drawToDefaultShaderProgram = nullptr;

void Framebuffer::initDrawToDefault()
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/framebuffer/drawToDefault");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/framebuffer/drawToDefault");
	
	_drawToDefaultShaderProgram = Engine::getGlobalRM().requestShaderProgram(createInfo);
}

void Framebuffer::drawToDefault(ShaderProgram* shader, bool clearFramebuffer)
{
	GLStateManager::push();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (clearFramebuffer)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	GLStateManager::setBlend(true);
	GLStateManager::setBlendEquation(GL_FUNC_ADD);
	GLStateManager::setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	shader->bind();
	
	RenderHelper::drawScreenQuad();
	
	GLStateManager::pop();
}

void Framebuffer::drawToDefault(const Texture& texture, bool clearFramebuffer)
{
	_drawToDefaultShaderProgram->setUniform("Texture", &texture);
	drawToDefault(_drawToDefaultShaderProgram, clearFramebuffer);
}

#pragma endregion DrawToDefault