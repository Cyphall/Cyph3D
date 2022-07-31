#pragma once

#include "Cyph3D/GLObject/BufferBase.h"

#include <glm/glm.hpp>
#include <array>
#include <optional>

class GLTexture;
class GLCubemap;

class GLFramebuffer : public BufferBase
{
public:
	explicit GLFramebuffer(glm::ivec2 size);
	
	~GLFramebuffer() override;
	
	void bindForDrawing();
	void bindForReading();
	glm::ivec2 getSize();
	
	void attachColor(int attachmentSlot, const GLTexture& texture, int level = 0);
	void attachColor(int attachmentSlot, const GLCubemap& cubemap, std::optional<int> face = std::nullopt, int level = 0);
	void detachColor(int attachmentSlot);
	
	void attachDepth(const GLTexture& texture, int level = 0);
	void attachDepth(const GLCubemap& cubemap, std::optional<int> face = std::nullopt, int level = 0);
	void detachDepth();
	
	void attachStencil(const GLTexture& texture, int level = 0);
	void attachStencil(const GLCubemap& cubemap, std::optional<int> face = std::nullopt, int level = 0);
	void detachStencil();
	
	void addToDrawBuffers(int attachmentSlot, int drawLocation);
	void removeFromDrawBuffers(int attachmentSlot);
	
	void setReadBuffer(int attachmentSlot);
	void removeReadBuffer();
	
private:
	glm::ivec2 _size;
	
	std::array<std::optional<int>, 32> _colorAttachments;
	std::optional<int> _readBuffer;
	
	bool _drawBuffersChanged = true;
	bool _readBufferChanged = true;
	
	void attachColorImpl(int attachmentSlot, GLuint handle, std::optional<int> face, int level);
	void detachColorImpl(int attachmentSlot);
	void attachDepthImpl(GLuint handle, std::optional<int> face, int level);
	void detachDepthImpl();
	void attachStencilImpl(GLuint handle, std::optional<int> face, int level);
	void detachStencilImpl();
	
	void addToDrawBuffersImpl(int attachmentSlot, int drawLocation);
	void removeFromDrawBuffersImpl(int attachmentSlot);
	
	void setReadBufferImpl(int attachmentSlot);
	void removeReadBufferImpl();
	
	void updateDrawBuffers();
	void updateReadBuffer();
	
	void checkDrawCompleteness();
	void checkReadCompleteness();
	
	void verifySize(glm::ivec2 size);
	void verifyDrawBufferCount();
	void verifyColorAttachmentSlots();
	void verifyFace(int face);
};