#pragma once

#include "Cyph3D/GLObject/GLBuffer.h"

template<typename T>
class GLImmutableBuffer : public GLBuffer<T>
{
public:
	GLImmutableBuffer(GLsizeiptr count, GLenum flags):
		GLBuffer<T>(count), _flags(flags)
	{
		glNamedBufferStorage(this->_handle, this->getSize(), nullptr, _flags);
	}

private:
	GLenum _flags;
};