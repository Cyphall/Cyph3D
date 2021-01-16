#include "Framebuffer.h"
#include <stdexcept>
#include <vector>
#include "../Helper/MathHelper.h"
#include "../Engine.h"
#include "../ResourceManagement/ResourceManager.h"
#include <fmt/core.h>
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

Framebuffer::ColorAttachment* Framebuffer::getColorAttachmentByHandle(GLuint handle)
{
	for (ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.handle == handle)
		{
			return &attachment;
		}
	}
	
	return nullptr;
}

void Framebuffer::checkDrawCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_DRAW_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(fmt::format("Framebuffer is incomplete for drawing: {}", state));
	}
}

void Framebuffer::checkReadCompleteness()
{
	GLenum state = glCheckNamedFramebufferStatus(_handle, GL_READ_FRAMEBUFFER);
	if (state != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error(fmt::format("Framebuffer is incomplete for reading: {}", state));
	}
}

void Framebuffer::verifySize(glm::ivec2 size)
{
	if (size != _size)
	{
		throw std::runtime_error("The texture size does not match the framebuffer size");
	}
}

void Framebuffer::verifyDrawBufferCount()
{
	int maxDrawBuffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	
	int drawBufferCount = 0;
	for (const ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.drawSlot.has_value())
		{
			drawBufferCount++;
		}
	}
	
	if (drawBufferCount > maxDrawBuffers)
	{
		throw std::runtime_error("The number of draw buffers is higher than the maximum number of draw buffers");
	}
}

void Framebuffer::verifyColorAttachmentCount()
{
	int maxColorAttachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
	
	if (_colorAttachments.size() >= maxColorAttachments)
	{
		throw std::runtime_error("The number color attachments is higher than the maximum number of color attachments");
	}
}

void Framebuffer::verifyFace(int face)
{
	if (!MathHelper::between(face, 0, 5))
	{
		throw std::runtime_error("The face index must be between 0 and 5 included");
	}
}

void Framebuffer::verifyColorAttachmentExistence(GLuint handle)
{
	for (const ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.handle == handle)
		{
			return;
		}
	}
	
	throw std::runtime_error("The texture is not attached to this framebuffer");
}

void Framebuffer::verifyObjectIsNotAttachedAsColor(GLuint handle)
{
	for (const ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.handle == handle)
		{
			throw std::runtime_error("This texture is already attached as color to this framebuffer");
		}
	}
}

void Framebuffer::verifyDepthAttachmentExistence()
{
	if (_depthAttachment == 0)
	{
		throw std::runtime_error("No texture is attached as depth to this framebuffer");
	}
}

void Framebuffer::verifyObjectIsNotAttachedAsDepth(GLuint handle)
{
	if (_depthAttachment == handle)
	{
		throw std::runtime_error("This texture is already attached as depth to this framebuffer");
	}
}

void Framebuffer::verifyStencilAttachmentExistence()
{
	if (_stencilAttachment == 0)
	{
		throw std::runtime_error("No texture is attached as stencil to this framebuffer");
	}
}

void Framebuffer::verifyObjectIsNotAttachedAsStencil(GLuint handle)
{
	if (_stencilAttachment == handle)
	{
		throw std::runtime_error("This texture is already attached as stencil to this framebuffer");
	}
}

void Framebuffer::verifyTextureIsDrawBuffer(GLuint handle)
{
	ColorAttachment* attachment = getColorAttachmentByHandle(handle);
	
	if (!attachment->drawSlot)
	{
		throw std::runtime_error("This texture is not in a draw buffer slot");
	}
}

void Framebuffer::verifyTextureIsNotDrawBuffer(GLuint handle)
{
	ColorAttachment* attachment = getColorAttachmentByHandle(handle);
	
	if (attachment->drawSlot)
	{
		throw std::runtime_error("This texture is already in a draw buffer slot");
	}
}

void Framebuffer::verifyTextureIsReadBuffer(GLuint handle)
{
	if (_readBuffer != handle)
	{
		throw std::runtime_error("This texture is not in a draw buffer slot");
	}
}

void Framebuffer::verifyTextureIsNotReadBuffer(GLuint handle)
{
	if (_readBuffer == handle)
	{
		throw std::runtime_error("This texture is already in a draw buffer slot");
	}
}

void Framebuffer::updateDrawBuffers()
{
	int highestIndex = -1;
	for (const ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.drawSlot.has_value() && attachment.drawSlot.value() > highestIndex)
		{
			highestIndex = attachment.drawSlot.value();
		}
	}
	
	std::vector<GLenum> drawBuffers;
	drawBuffers.resize(highestIndex+1, GL_NONE);
	
	for (const ColorAttachment& attachment : _colorAttachments)
	{
		if (attachment.drawSlot.has_value())
		{
			drawBuffers[attachment.drawSlot.value()] = GL_COLOR_ATTACHMENT0 + attachment.attachmentSlot;
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

#pragma region Attachment setters

void Framebuffer::attachColor(const Texture& texture)
{
	verifySize(texture.getSize());
	verifyObjectIsNotAttachedAsColor(texture.getHandle());
	verifyObjectIsNotAttachedAsDepth(texture.getHandle());
	verifyObjectIsNotAttachedAsStencil(texture.getHandle());
	
	attachColorImpl(texture.getHandle(), -1);
}

void Framebuffer::attachColor(const Cubemap& cubemap)
{
	verifySize(cubemap.getSize());
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsDepth(cubemap.getHandle());
	verifyObjectIsNotAttachedAsStencil(cubemap.getHandle());
	
	attachColorImpl(cubemap.getHandle(), -1);
}

void Framebuffer::attachColor(const Cubemap& cubemap, int face)
{
	verifySize(cubemap.getSize());
	verifyFace(face);
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsDepth(cubemap.getHandle());
	verifyObjectIsNotAttachedAsStencil(cubemap.getHandle());
	
	attachColorImpl(cubemap.getHandle(), face);
}

void Framebuffer::detachColor(const Texture& texture)
{
	verifyColorAttachmentExistence(texture.getHandle());
	
	detachColorImpl(texture.getHandle());
}

void Framebuffer::detachColor(const Cubemap& cubemap)
{
	verifyColorAttachmentExistence(cubemap.getHandle());
	
	detachColorImpl(cubemap.getHandle());
}

void Framebuffer::attachDepth(const Texture& texture)
{
	if (_depthAttachment == texture.getHandle())
		return;
	
	verifySize(texture.getSize());
	verifyObjectIsNotAttachedAsColor(texture.getHandle());
	verifyObjectIsNotAttachedAsDepth(texture.getHandle());
	
	attachDepthImpl(texture.getHandle(), -1);
}

void Framebuffer::attachDepth(const Cubemap& cubemap)
{
	if (_depthAttachment == cubemap.getHandle())
		return;
	
	verifySize(cubemap.getSize());
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsDepth(cubemap.getHandle());
	
	attachDepthImpl(cubemap.getHandle(), -1);
}

void Framebuffer::attachDepth(const Cubemap& cubemap, int face)
{
	if (_depthAttachment == cubemap.getHandle())
		return;
	
	verifySize(cubemap.getSize());
	verifyFace(face);
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsDepth(cubemap.getHandle());
	
	attachDepthImpl(cubemap.getHandle(), face);
}

void Framebuffer::detachDepth()
{
	verifyDepthAttachmentExistence();
	
	detachDepthImpl();
}

void Framebuffer::attachStencil(const Texture& texture)
{
	if (_stencilAttachment == texture.getHandle())
		return;
	
	verifySize(texture.getSize());
	verifyObjectIsNotAttachedAsColor(texture.getHandle());
	verifyObjectIsNotAttachedAsStencil(texture.getHandle());
	
	attachStencilImpl(texture.getHandle(), -1);
}

void Framebuffer::attachStencil(const Cubemap& cubemap)
{
	if (_stencilAttachment == cubemap.getHandle())
		return;
	
	verifySize(cubemap.getSize());
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsStencil(cubemap.getHandle());
	
	attachStencilImpl(cubemap.getHandle(), -1);
}

void Framebuffer::attachStencil(const Cubemap& cubemap, int face)
{
	if (_stencilAttachment == cubemap.getHandle())
		return;
	
	verifySize(cubemap.getSize());
	verifyFace(face);
	verifyObjectIsNotAttachedAsColor(cubemap.getHandle());
	verifyObjectIsNotAttachedAsStencil(cubemap.getHandle());
	
	attachStencilImpl(cubemap.getHandle(), face);
}

void Framebuffer::detachStencil()
{
	verifyStencilAttachmentExistence();
	
	detachStencilImpl();
}

void Framebuffer::attachColorImpl(GLuint handle, int face)
{
	int slot;
	for (slot = 0;; slot++)
	{
		bool alreadyPresent = false;
		for (const ColorAttachment& attachment : _colorAttachments)
		{
			if (attachment.attachmentSlot == slot)
			{
				alreadyPresent = true;
				break;
			}
		}
		
		if (!alreadyPresent)
		{
			break;
		}
	}
	
	GLenum attachmentEnum = GL_COLOR_ATTACHMENT0 + slot;
	
	if (face == -1)
	{
		glNamedFramebufferTexture(_handle, attachmentEnum, handle, 0);
		
	}
	else
	{
		glNamedFramebufferTextureLayer(_handle, attachmentEnum, handle, 0, face);
	}
	
	ColorAttachment attachment{};
	attachment.handle = handle;
	attachment.attachmentSlot = slot;
	_colorAttachments.push_back(attachment);
	
	verifyColorAttachmentCount();
}

void Framebuffer::detachColorImpl(GLuint handle)
{
	for (auto it = _colorAttachments.cbegin(); it != _colorAttachments.cend(); )
	{
		if (it->handle == handle)
		{
			GLenum attachmentEnum = GL_COLOR_ATTACHMENT0 + it->attachmentSlot;
			glNamedFramebufferTexture(_handle, attachmentEnum, 0, 0);
			
			if (it->drawSlot.has_value())
			{
				_drawBuffersChanged = true;
			}
			
			it = _colorAttachments.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Framebuffer::attachDepthImpl(GLuint handle, int face)
{
	if (face == -1)
	{
		glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, handle, 0);
		
	}
	else
	{
		glNamedFramebufferTextureLayer(_handle, GL_DEPTH_ATTACHMENT, handle, 0, face);
	}
	
	_depthAttachment = handle;
}

void Framebuffer::detachDepthImpl()
{
	glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, 0, 0);
	
	_depthAttachment.reset();
}

void Framebuffer::attachStencilImpl(GLuint handle, int face)
{
	if (face == -1)
	{
		glNamedFramebufferTexture(_handle, GL_STENCIL_ATTACHMENT, handle, 0);
		
	}
	else
	{
		glNamedFramebufferTextureLayer(_handle, GL_STENCIL_ATTACHMENT, handle, 0, face);
	}
	
	_stencilAttachment = handle;
}

void Framebuffer::detachStencilImpl()
{
	glNamedFramebufferTexture(_handle, GL_STENCIL_ATTACHMENT, 0, 0);
	
	_stencilAttachment.reset();
}

void Framebuffer::addToDrawBuffersImpl(GLuint handle, int slot)
{
	ColorAttachment* attachment = getColorAttachmentByHandle(handle);
	
	attachment->drawSlot = slot;
	
	_drawBuffersChanged = true;
}

void Framebuffer::removeFromDrawBuffersImpl(GLuint handle)
{
	ColorAttachment* attachment = getColorAttachmentByHandle(handle);
	
	attachment->drawSlot.reset();
	
	_drawBuffersChanged = true;
}

void Framebuffer::setReadBufferImpl(GLuint handle)
{
	_readBuffer = getColorAttachmentByHandle(handle)->attachmentSlot;
}

void Framebuffer::removeReadBufferImpl()
{
	_readBuffer.reset();
}

#pragma endregion Attachment setters

#pragma region Draw buffers setters

void Framebuffer::addToDrawBuffers(const Texture& texture, int slot)
{
	verifyColorAttachmentExistence(texture.getHandle());
	verifyTextureIsNotDrawBuffer(texture.getHandle());
	
	addToDrawBuffersImpl(texture.getHandle(), slot);
	
	verifyDrawBufferCount();
}

void Framebuffer::addToDrawBuffers(const Cubemap& cubemap, int slot)
{
	verifyColorAttachmentExistence(cubemap.getHandle());
	verifyTextureIsNotDrawBuffer(cubemap.getHandle());
	
	addToDrawBuffersImpl(cubemap.getHandle(), slot);
	
	verifyDrawBufferCount();
}

void Framebuffer::removeFromDrawBuffers(const Texture& texture)
{
	verifyColorAttachmentExistence(texture.getHandle());
	verifyTextureIsDrawBuffer(texture.getHandle());
	
	removeFromDrawBuffersImpl(texture.getHandle());
}

void Framebuffer::removeFromDrawBuffers(const Cubemap& cubemap)
{
	verifyColorAttachmentExistence(cubemap.getHandle());
	verifyTextureIsDrawBuffer(cubemap.getHandle());
	
	removeFromDrawBuffersImpl(cubemap.getHandle());
}

void Framebuffer::setReadBuffer(const Texture& texture)
{
	verifyColorAttachmentExistence(texture.getHandle());
	verifyTextureIsNotReadBuffer(texture.getHandle());
	
	setReadBufferImpl(texture.getHandle());
}

void Framebuffer::setReadBuffer(const Cubemap& cubemap)
{
	verifyColorAttachmentExistence(cubemap.getHandle());
	verifyTextureIsNotReadBuffer(cubemap.getHandle());
	
	setReadBufferImpl(cubemap.getHandle());
}

void Framebuffer::removeReadBuffer()
{
	if (!_readBuffer)
	{
		throw std::runtime_error("No texture is defined as read buffer");
	}
	
	removeReadBufferImpl();
}

#pragma endregion Draw buffers setters

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
	int previousBlend; glGetIntegerv(GL_BLEND, &previousBlend);
	int previousBlendSrcRgb; glGetIntegerv(GL_BLEND_SRC_RGB, &previousBlendSrcRgb);
	int previousBlendSrcAlpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, &previousBlendSrcAlpha);
	int previousBlendDstRgb; glGetIntegerv(GL_BLEND_DST_RGB, &previousBlendDstRgb);
	int previousBlendDstAlpha; glGetIntegerv(GL_BLEND_DST_ALPHA, &previousBlendDstAlpha);
	int previousBlendEquationRgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &previousBlendEquationRgb);
	int previousBlendEquationAlpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &previousBlendEquationAlpha);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (clearFramebuffer)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	shader->bind();
	
	RenderHelper::drawScreenQuad();
	
	if (previousBlend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	glBlendFuncSeparate(previousBlendSrcRgb, previousBlendDstRgb, previousBlendSrcAlpha, previousBlendDstAlpha);
	glBlendEquationSeparate(previousBlendEquationRgb, previousBlendEquationAlpha);
}

void Framebuffer::drawToDefault(const Texture& texture, bool clearFramebuffer)
{
	_drawToDefaultShaderProgram->setUniform("u_texture", &texture);
	drawToDefault(_drawToDefaultShaderProgram, clearFramebuffer);
}

#pragma endregion DrawToDefault