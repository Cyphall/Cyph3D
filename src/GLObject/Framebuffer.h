#pragma once

#include "BufferBase.h"
#include "CreateInfo/TextureCreateInfo.h"
#include "CreateInfo/CubemapCreateInfo.h"
#include "Texture.h"
#include "Cubemap.h"
#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <set>

class Framebuffer : public BufferBase
{
public:
	explicit Framebuffer(glm::ivec2 size);
	
	~Framebuffer() override;
	
	void clearAll();
	void clearColor();
	void clearDepth();
	void clearStencil();
	
	void bind();
	glm::ivec2 getSize();
	
	void attach(GLenum attachment, const Texture& texture);
	void attach(GLenum attachment, const Cubemap& cubemap);
	void attach(GLenum attachment, const Cubemap& cubemap, int face);
	void detach(GLenum attachment);
	
	static void initDrawToDefault();
	
	static void drawToDefault(ShaderProgram* shader, bool clearFramebuffer = false);
	static void drawToDefault(const Texture& texture, bool clearFramebuffer = false);
	
private:
	glm::ivec2 _size;
	std::set<GLenum> _drawBuffers;
	
	void checkState();
	
	static bool isDrawBuffer(GLenum attachment);

	static ShaderProgram* _drawToDefaultShaderProgram;
};
