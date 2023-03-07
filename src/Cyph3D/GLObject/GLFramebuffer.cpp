#include "GLFramebuffer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/Helper/RenderHelper.h"

#include <format>
#include <stdexcept>
#include <vector>

GLFramebuffer::GLFramebuffer()
{
	glCreateFramebuffers(1, &_handle);
}

GLFramebuffer::~GLFramebuffer()
{
	glDeleteFramebuffers(1, &_handle);
	_handle = 0;
}

void GLFramebuffer::bindForDrawing()
{
	if (_drawBuffersChanged)
	{
		updateDrawBuffers();
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _handle);
}

void GLFramebuffer::bindForReading()
{
	if (_readBufferChanged)
	{
		updateReadBuffer();
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _handle);
}

void GLFramebuffer::checkDrawCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_DRAW_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(std::format("Framebuffer is incomplete for drawing: {}", state));
	}
}

void GLFramebuffer::checkReadCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_READ_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(std::format("Framebuffer is incomplete for reading: {}", state));
	}
}

void GLFramebuffer::verifyDrawBufferCount()
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

void GLFramebuffer::verifyColorAttachmentSlots()
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

void GLFramebuffer::verifyFace(int face)
{
	if (!MathHelper::between(face, 0, 5))
	{
		throw std::runtime_error(std::format("The face index ({}) is not in the interval [0,5]", face));
	}
}

void GLFramebuffer::updateDrawBuffers()
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

void GLFramebuffer::updateReadBuffer()
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

void GLFramebuffer::attachColor(int attachmentSlot, const GLTexture& texture, int level)
{
	attachColorImpl(attachmentSlot, texture.getHandle(), std::nullopt, level);
	
	verifyColorAttachmentSlots();
}

void GLFramebuffer::attachColor(int attachmentSlot, const GLCubemap& cubemap, std::optional<int> face, int level)
{
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachColorImpl(attachmentSlot, cubemap.getHandle(), face, level);
	
	verifyColorAttachmentSlots();
}

void GLFramebuffer::detachColor(int attachmentSlot)
{
	detachColorImpl(attachmentSlot);
}

void GLFramebuffer::attachDepth(const GLTexture& texture, int level)
{
	attachDepthImpl(texture.getHandle(), std::nullopt, level);
}

void GLFramebuffer::attachDepth(const GLCubemap& cubemap, std::optional<int> face, int level)
{
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachDepthImpl(cubemap.getHandle(), face, level);
}

void GLFramebuffer::detachDepth()
{
	detachDepthImpl();
}

void GLFramebuffer::attachStencil(const GLTexture& texture, int level)
{
	attachStencilImpl(texture.getHandle(), std::nullopt, level);
}

void GLFramebuffer::attachStencil(const GLCubemap& cubemap, std::optional<int> face, int level)
{
	if (face.has_value())
	{
		verifyFace(face.value());
	}
	
	attachStencilImpl(cubemap.getHandle(), face, level);
}

void GLFramebuffer::detachStencil()
{
	detachStencilImpl();
}

void GLFramebuffer::attachColorImpl(int attachmentSlot, GLuint handle, std::optional<int> face, int level)
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

void GLFramebuffer::detachColorImpl(int attachmentSlot)
{
	glNamedFramebufferTexture(_handle, GL_COLOR_ATTACHMENT0 + attachmentSlot, 0, 0);
}

void GLFramebuffer::attachDepthImpl(GLuint handle, std::optional<int> face, int level)
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

void GLFramebuffer::detachDepthImpl()
{
	glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, 0, 0);
}

void GLFramebuffer::attachStencilImpl(GLuint handle, std::optional<int> face, int level)
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

void GLFramebuffer::detachStencilImpl()
{
	glNamedFramebufferTexture(_handle, GL_STENCIL_ATTACHMENT, 0, 0);
}

void GLFramebuffer::addToDrawBuffersImpl(int attachmentSlot, int drawLocation)
{
	_colorAttachments[attachmentSlot] = drawLocation;
	_drawBuffersChanged = true;
}

void GLFramebuffer::removeFromDrawBuffersImpl(int attachmentSlot)
{
	_colorAttachments[attachmentSlot].reset();
	_drawBuffersChanged = true;
}

void GLFramebuffer::setReadBufferImpl(int attachmentSlot)
{
	_readBuffer = attachmentSlot;
	_readBufferChanged = true;
}

void GLFramebuffer::removeReadBufferImpl()
{
	_readBuffer.reset();
	_readBufferChanged = true;
}

void GLFramebuffer::addToDrawBuffers(int attachmentSlot, int drawLocation)
{
	addToDrawBuffersImpl(attachmentSlot, drawLocation);
	
	verifyDrawBufferCount();
}

void GLFramebuffer::removeFromDrawBuffers(int attachmentSlot)
{
	removeFromDrawBuffersImpl(attachmentSlot);
}

void GLFramebuffer::setReadBuffer(int attachmentSlot)
{
	setReadBufferImpl(attachmentSlot);
}

void GLFramebuffer::removeReadBuffer()
{
	removeReadBufferImpl();
}