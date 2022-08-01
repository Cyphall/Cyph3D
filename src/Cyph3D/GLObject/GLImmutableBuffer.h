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

	T* map(GLbitfield access)
	{
		return static_cast<T*>(glMapNamedBufferRange(this->_handle, 0, this->getSize(), access));
	}

	void unmap()
	{
		glUnmapNamedBuffer(this->_handle);
	}

private:
	GLenum _flags;
};