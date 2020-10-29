#pragma once

#include "Buffer.h"

template<typename T>
class ShaderStorageBuffer : public Buffer<T>
{
public:
	explicit ShaderStorageBuffer(GLenum usage = GL_DYNAMIC_DRAW):
	Buffer<T>(usage)
	{
	
	}
	
	void bind(int bindingPoint)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, this->_handle);
	}
};