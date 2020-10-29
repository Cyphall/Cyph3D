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
		
		glNamedBufferStorage(_handle, _count * sizeof(T), nullptr, _immutableFlags);
	}
	
	~Buffer() override
	{
		glDeleteBuffers(1, &_handle);
	}
	
	void setData(const T* data, int dataCount)
	{
		if (_isMutable)
		{
			_count = dataCount;
			glNamedBufferData(_handle, _count * sizeof(T), data, _mutableUsage);
		}
		else
		{
			assert(dataCount == _count);
			glNamedBufferSubData(_handle, 0, _count * sizeof(T), data);
		}
	}
	
	void setData(const std::vector<T>& data)
	{
		setData(data.data(), data.size());
	}
	
	GLsizeiptr getCount()
	{
		return _count;
	}

private:
	bool _isMutable;
	GLsizeiptr _count = -1;
	GLenum _mutableUsage;
	GLenum _immutableFlags;
};


