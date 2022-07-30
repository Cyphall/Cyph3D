#pragma once

#include "Cyph3D/GLObject/BufferBase.h"
#include <span>
#include <cassert>

template<typename T>
class Buffer : public BufferBase
{
public:
	explicit Buffer(GLenum usage):
	_isMutable(true), _mutableUsage(usage)
	{
		glCreateBuffers(1, &_handle);
	}
	
	Buffer(GLsizeiptr count, GLenum flags):
	_isMutable(false), _count(count), _immutableFlags(flags)
	{
		glCreateBuffers(1, &_handle);
		
		glNamedBufferStorage(_handle, getSize(), nullptr, _immutableFlags);
	}
	
	~Buffer() override
	{
		glDeleteBuffers(1, &_handle);
	}
	
	void resize(GLsizeiptr elementCount)
	{
		assert(_isMutable);
		_count = elementCount;
		glNamedBufferData(_handle, getSize(), nullptr, _mutableUsage);
	}
	
	void setData(std::span<const T> data)
	{
		if (_isMutable)
		{
			_count = data.size();
			glNamedBufferData(_handle, getSize(), data.data(), _mutableUsage);
		}
		else
		{
			assert(data.size() == _count);
			glNamedBufferSubData(_handle, 0, getSize(), data.data());
		}
	}
	
	GLsizeiptr getCount() const
	{
		return _count;
	}
	
	GLsizeiptr getSize() const
	{
		return _count * sizeof(T);
	}

private:
	bool _isMutable;
	GLsizeiptr _count = -1;
	GLenum _mutableUsage = 0;
	GLenum _immutableFlags = 0;
};

