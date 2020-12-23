#pragma once

#include <map>
#include "BufferBase.h"
#include "Buffer.h"

class VertexArray : public BufferBase
{
public:
	VertexArray()
	{
		glCreateVertexArrays(1, &_handle);
	}
	
	~VertexArray() override
	{
		glDeleteVertexArrays(1, &_handle);
	}
	
	void defineFormat(int bufferSlot, int attribLocation, int elementCount, GLenum elementType, int offset)
	{
		glVertexArrayAttribBinding(_handle, attribLocation, bufferSlot); // sets which buffer slot will be used for this vertex attribute
		glEnableVertexArrayAttrib(_handle, attribLocation); // enables this vertex attribute
		glVertexArrayAttribFormat(_handle, attribLocation, elementCount, elementType, false, offset); // defines this vertex attribute
	}
	
	template<typename T>
	void bindBufferToSlot(const Buffer<T>& buffer, int bufferSlot)
	{
		glVertexArrayVertexBuffer(_handle, bufferSlot, buffer.getHandle(), 0, sizeof(T));
	}
	
	void bindIndexBuffer(const Buffer<GLuint>& buffer)
	{
		glVertexArrayElementBuffer(_handle, buffer.getHandle());
	}
	
	void bind()
	{
		glBindVertexArray(_handle);
	}
};


