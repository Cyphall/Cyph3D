#pragma once

#include "Cyph3D/GLObject/GLObject.h"

template<typename T>
class GLBuffer;

class GLVertexArray : public GLObject
{
public:
	GLVertexArray();
	~GLVertexArray() override;
	
	void defineFormat(int bufferSlot, int attribLocation, int elementCount, GLenum elementType, int offset);
	
	template<typename T>
	void bindBufferToSlot(const GLBuffer<T>& buffer, int bufferSlot)
	{
		glVertexArrayVertexBuffer(_handle, bufferSlot, buffer.getHandle(), 0, sizeof(T));
	}
	
	void bindIndexBuffer(const GLBuffer<GLuint>& buffer);
	
	void bind();
};