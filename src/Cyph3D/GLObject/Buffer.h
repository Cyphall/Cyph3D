#pragma once

#include "BufferBase.h"
#include <vector>
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
	
	void setData(const T* data, GLsizeiptr dataCount)
	{
		if (_isMutable)
		{
			_count = dataCount;
			glNamedBufferData(_handle, getSize(), data, _mutableUsage);
		}
		else
		{
			assert(dataCount == _count);
			glNamedBufferSubData(_handle, 0, getSize(), data);
		}
	}
	
	void setData(const std::vector<T>& data)
	{
		setData(data.data(), data.size());
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


