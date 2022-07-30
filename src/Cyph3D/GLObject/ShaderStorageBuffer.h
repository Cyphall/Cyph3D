#pragma once

#include "Cyph3D/GLObject/Buffer.h"

template<typename T>
class ShaderStorageBuffer : public Buffer<T>
{
public:
	explicit ShaderStorageBuffer(GLenum usage = GL_DYNAMIC_DRAW):
	Buffer<T>(usage)
	{
	
	}
	
	ShaderStorageBuffer(GLsizeiptr count, GLenum flags):
	Buffer<T>(count, flags)
	{
	
	}
	
	void bind(int bindingPoint)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, this->_handle);
	}
};