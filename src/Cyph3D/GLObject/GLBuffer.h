#pragma once

#include "Cyph3D/GLObject/GLObject.h"

#include <span>
#include <stdexcept>

template<typename T>
class GLBuffer : public GLObject
{
public:
	~GLBuffer()
	{
		glDeleteBuffers(1, &_handle);
		_handle = 0;
	}
	
	void bind(GLenum target)
	{
		glBindBuffer(target, _handle);
	}
	
	void bindBase(GLenum target, GLuint index)
	{
		glBindBufferBase(target, index, _handle);
	}
	
	GLsizeiptr getCount() const
	{
		return _count;
	}
	
	GLsizeiptr getSize() const
	{
		return _count * sizeof(T);
	}
	
	void setData(std::span<const T> data, GLintptr offset = 0)
	{
		if (offset + data.size() > _count)
		{
			throw std::out_of_range("The buffer is too short for this copy operation.");
		}
		glNamedBufferSubData(_handle, offset * sizeof(T), data.size() * sizeof(T), data.data());
	}

	T* map(GLbitfield access)
	{
		return static_cast<T*>(glMapNamedBufferRange(this->_handle, 0, this->getSize(), access));
	}

	void unmap()
	{
		glUnmapNamedBuffer(this->_handle);
	}
	
protected:
	GLsizeiptr _count = 0;

	explicit GLBuffer(GLsizeiptr count):
		_count(count)
	{
		glCreateBuffers(1, &this->_handle);
	}
};