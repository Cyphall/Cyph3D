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
	
private:
	GLenum _usage;
};