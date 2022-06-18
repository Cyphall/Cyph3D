#include "Framebuffer.h"
#include <stdexcept>
#include <vector>
#include "../Helper/MathHelper.h"
#include "../Engine.h"
#include "../ResourceManagement/ResourceManager.h"
#include <format>
#include "../Helper/RenderHelper.h"

Framebuffer::Framebuffer(glm::ivec2 size):
_size(size)
{
	glCreateFramebuffers(1, &_handle);
	
	glNamedFramebufferParameteri(_handle, GL_FRAMEBUFFER_DEFAULT_WIDTH, _size.x);
	glNamedFramebufferParameteri(_handle, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _size.y);
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &_handle);
}

void Framebuffer::bindForDrawing()
{
	if (_drawBuffersChanged)
	{
		updateDrawBuffers();
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _handle);
}

void Framebuffer::bindForReading()
{
	if (_readBufferChanged)
	{
		updateReadBuffer();
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _handle);
}

glm::ivec2 Framebuffer::getSize()
{
	return _size;
}

void Framebuffer::checkDrawCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_DRAW_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(std::format("Framebuffer is incomplete for drawing: {}", state));
	}
}

void Framebuffer::checkReadCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_READ_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(std::format("Framebuffer is incomplete for reading: {}", state));
	}
}

void Framebuffer::verifySize(glm::ivec2 size)
{
	if (size != _size)
	{
		throw std::runtime_error(std::format("The texture size ({}x{}) does not match the framebuffer size ({}x{})", size.x, size.y, _size.x, _size.y));
	}
}

void Framebuffer::verifyDrawBufferCount()
{
	int maxDrawBuffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

	int drawBufferCount = 0;
	for (const std::optional<int>& attachment : _colorAttachments)
	{
		if (attachment.has_value())
		{
			drawBufferCount++;
		}
	}

	if (drawBufferCount > maxDrawBuffers)
	{
		throw std::runtime_error(std::format("The number of draw buffers ({}) is higher than the maximum number of draw buffers ({})", drawBufferCount, maxDrawBuffers));
	}
}

void Framebuffer::verifyColorAttachmentSlots()
{
	int maxColorAttachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	
	for (int i = maxColorAttachments; i < 32; i++)
	{
		const std::optional<int>& attachment = _colorAttachments[i];
		if (attachment.has_value())
		{
			throw std::runtime_error(std::format("The color attachment slot {} is above the highest available slot ({})", i, maxColorAttachments));
		}
	}
}

void Framebuffer::verifyFace(int face)
{
	if (!MathHelper::between(face, 0, 5))
	{
		throw std::runtime_error(std::format("The face index ({}) is not in the interval [0,5]", face));
	}
}

void Framebuffer::updateDrawBuffers()
{
	std::optional<int> highestIndex;
	for (const std::optional<int>& attachment : _colorAttachments)
	{
		if (attachment.has_value() && (!highestIndex.has_value() || attachment.value() > highestIndex))
		{
			highestIndex = attachment.value();
		}
	}
	
	std::vector<GLenum> drawBuffers;
	if (highestIndex.has_value())
	{
		drawBuffers.resize(highestIndex.value() + 1, GL_NONE);
		
		for (int i = 0; i < 32; i++)
		{
			const std::optional<int>& attachment = _colorAttachments[i];
			
			if (attachment.has_value())
			{
				drawBuffers[attachment.value()] = GL_COLOR_ATTACHMENT0 + i;
			}
		}
	}
	
	glNamedFramebufferDrawBuffers(_handle, drawBuffers.size(), drawBuffers.data());
	
	checkDrawCompleteness();
	_drawBuffersChanged = false;
}

void Framebuffer::updateReadBuffer()
{
	if (_readBuffer.has_value())
	{
		glNamedFramebufferReadBuffer(_handle, GL_COLOR_ATTACHMENT0 + _readBuffer.value());
	}
	else
	{
		glNamedFramebufferReadBuffer(_handle, GL_NONE);
	}
	
	checkReadCompleteness();
	_readBufferChanged = false;
}

void Framebuffer::attachColor(int attachmentSlot, const Texture& texture, int level)
{
	verifySize(texture.getSize(level));
	
	attachColorImpl(attachmentSlot, texture.getHandle(), std::nullopt, level);
	
	verifyColorAttachmentSlots();
}

void Framebuffer::attachColor(int attachmentSlot, const Cubemap& cubemap, std::optional<int> face, int level)
{
	verifySize(cubemap.getSize());
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachColorImpl(attachmentSlot, cubemap.getHandle(), face, level);
	
	verifyColorAttachmentSlots();
}

void Framebuffer::detachColor(int attachmentSlot)
{
	detachColorImpl(attachmentSlot);
}

void Framebuffer::attachDepth(const Texture& texture, int level)
{
	verifySize(texture.getSize(level));
	
	attachDepthImpl(texture.getHandle(), std::nullopt, level);
}

void Framebuffer::attachDepth(const Cubemap& cubemap, std::optional<int> face, int level)
{
	verifySize(cubemap.getSize());
	
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachDepthImpl(cubemap.getHandle(), face, level);
}

void Framebuffer::detachDepth()
{
	detachDepthImpl();
}

void Framebuffer::attachStencil(const Texture& texture, int level)
{
	verifySize(texture.getSize(level));
	
	attachStencilImpl(texture.getHandle(), std::nullopt, level);
}

void Framebuffer::attachStencil(const Cubemap& cubemap, std::optional<int> face, int level)
{
	verifySize(cubemap.getSize());
	
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachStencilImpl(cubemap.getHandle(), face, level);
}

void Framebuffer::detachStencil()
{
	detachStencilImpl();
}

void Framebuffer::attachColorImpl(int attachmentSlot, GLuint handle, std::optional<int> face, int level)
{
	GLenum attachmentEnum = GL_COLOR_ATTACHMENT0 + attachmentSlot;
	
	if (face)
	{
		glNamedFramebufferTextureLayer(_handle, attachmentEnum, handle, level, face.value());
	}
	else
	{
		glNamedFramebufferTexture(_handle, attachmentEnum, handle, level);
	}
}

void Framebuffer::detachColorImpl(int attachmentSlot)
{
	glNamedFramebufferTexture(_handle, GL_COLOR_ATTACHMENT0 + attachmentSlot, 0, 0);
}

void Framebuffer::attachDepthImpl(GLuint handle, std::optional<int> face, int level)
{
	if (face)
	{
		glNamedFramebufferTextureLayer(_handle, GL_DEPTH_ATTACHMENT, handle, level, face.value());
	}
	else
	{
		glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, handle, level);
	}
}

void Framebuffer::detachDepthImpl()
{
	glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, 0, 0);
}

void Framebuffer::attachStencilImpl(GLuint handle, std::optional<int> face, int level)
{
	if (face)
	{
		glNamedFramebufferTextureLayer(_handle, GL_STENCIL_ATTACHMENT, handle, level, face.value());
	}
	else
	{
		glNamedFramebufferTexture(_handle, GL_STENCIL_ATTACHMENT, handle, level);
	}
}

void Framebuffer::detachStencilImpl()
{
	glNamedFramebufferTexture(_handle, GL_STENCIL_ATTACHMENT, 0, 0);
}

void Framebuffer::addToDrawBuffersImpl(int attachmentSlot, int drawLocation)
{
	_colorAttachments[attachmentSlot] = drawLocation;
	_drawBuffersChanged = true;
}

void Framebuffer::removeFromDrawBuffersImpl(int attachmentSlot)
{
	_colorAttachments[attachmentSlot].reset();
	_drawBuffersChanged = true;
}

void Framebuffer::setReadBufferImpl(int attachmentSlot)
{
	_readBuffer = attachmentSlot;
	_readBufferChanged = true;
}

void Framebuffer::removeReadBufferImpl()
{
	_readBuffer.reset();
	_readBufferChanged = true;
}

void Framebuffer::addToDrawBuffers(int attachmentSlot, int drawLocation)
{
	addToDrawBuffersImpl(attachmentSlot, drawLocation);
	
	verifyDrawBufferCount();
}

void Framebuffer::removeFromDrawBuffers(int attachmentSlot)
{
	removeFromDrawBuffersImpl(attachmentSlot);
}

void Framebuffer::setReadBuffer(int attachmentSlot)
{
	setReadBufferImpl(attachmentSlot);
}

void Framebuffer::removeReadBuffer()
{
	removeReadBufferImpl();
}
