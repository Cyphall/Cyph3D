#pragma once

#include "BufferBase.h"
#include "CreateInfo/TextureCreateInfo.h"
#include "CreateInfo/CubemapCreateInfo.h"
#include "Texture.h"
#include "Cubemap.h"
#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <list>

class Framebuffer : public BufferBase
{
public:
	struct ColorAttachment
	{
		GLuint             handle;
		int                attachmentSlot;
		std::optional<int> drawSlot;
	};
	
	explicit Framebuffer(glm::ivec2 size);
	
	~Framebuffer() override;
	
	void bindForDrawing();
	void bindForReading();
	glm::ivec2 getSize();
	
	void attachColor(const Texture& texture);
	void attachColor(const Cubemap& cubemap);
	void attachColor(const Cubemap& cubemap, int face);
	void detachColor(const Texture& texture);
	void detachColor(const Cubemap& cubemap);
	
	void attachDepth(const Texture& texture);
	void attachDepth(const Cubemap& cubemap);
	void attachDepth(const Cubemap& cubemap, int face);
	void detachDepth();
	
	void attachStencil(const Texture& texture);
	void attachStencil(const Cubemap& cubemap);
	void attachStencil(const Cubemap& cubemap, int face);
	void detachStencil();
	
	void addToDrawBuffers(const Texture& texture, int slot);
	void addToDrawBuffers(const Cubemap& cubemap, int slot);
	void removeFromDrawBuffers(const Texture& texture);
	void removeFromDrawBuffers(const Cubemap& cubemap);
	
	void setReadBuffer(const Texture& texture);
	void setReadBuffer(const Cubemap& cubemap);
	void removeReadBuffer();
	
	static void initDrawToDefault();
	
	static void drawToDefault(ShaderProgram* shader, bool clearFramebuffer = false);
	static void drawToDefault(const Texture& texture, bool clearFramebuffer = false);
	
private:
	glm::ivec2 _size;
	
	bool _drawBuffersChanged = true;
	bool _readBufferChanged = true;
	
	std::list<ColorAttachment> _colorAttachments;
	
	// GLuint: texture or cubemap handle
	std::optional<GLuint> _depthAttachment = 0;
	std::optional<GLuint> _stencilAttachment = 0;
	
	std::optional<int> _readBuffer;
	
	void verifySize(glm::ivec2 size);
	void verifyDrawBufferCount();
	void verifyColorAttachmentCount();
	void verifyFace(int face);
	void verifyColorAttachmentExistence(GLuint handle);
	void verifyObjectIsNotAttachedAsColor(GLuint handle);
	void verifyDepthAttachmentExistence();
	void verifyObjectIsNotAttachedAsDepth(GLuint handle);
	void verifyStencilAttachmentExistence();
	void verifyObjectIsNotAttachedAsStencil(GLuint handle);
	void verifyTextureIsDrawBuffer(GLuint handle);
	void verifyTextureIsNotDrawBuffer(GLuint handle);
	void verifyTextureIsReadBuffer(GLuint handle);
	void verifyTextureIsNotReadBuffer(GLuint handle);
	
	void attachColorImpl(GLuint handle, int face);
	void detachColorImpl(GLuint handle);
	void attachDepthImpl(GLuint handle, int face);
	void detachDepthImpl();
	void attachStencilImpl(GLuint handle, int face);
	void detachStencilImpl();
	
	void addToDrawBuffersImpl(GLuint handle, int slot);
	void removeFromDrawBuffersImpl(GLuint handle);
	
	void setReadBufferImpl(GLuint handle);
	void removeReadBufferImpl();
	
	ColorAttachment* getColorAttachmentByHandle(GLuint handle);
	
	void updateDrawBuffers();
	void updateReadBuffer();
	
	void checkDrawCompleteness();
	void checkReadCompleteness();

	static ShaderProgram* _drawToDefaultShaderProgram;
};
