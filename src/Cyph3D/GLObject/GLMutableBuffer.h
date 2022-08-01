#pragma once

#include "Cyph3D/GLObject/GLBuffer.h"

template<typename T>
class GLMutableBuffer : public GLBuffer<T>
{
public:
	explicit GLMutableBuffer(GLenum usage):
		GLBuffer<T>(0), _usage(usage)
	{}
	
	void resize(GLsizeiptr count)
	{
		this->_count = count;
		glNamedBufferData(this->_handle, this->getSize(), nullptr, _usage);
	}

	T* map(GLenum access)
	{
		return static_cast<T*>(glMapNamedBuffer(this->_handle, access));
	}

	void unmap()
	{
		glUnmapNamedBuffer(this->_handle);
	}
	
private:
	GLenum _usage;
};