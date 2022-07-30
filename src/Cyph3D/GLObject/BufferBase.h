#pragma once

#include <glad/glad.h>

class BufferBase
{
public:
	BufferBase() = default;
	BufferBase(const BufferBase& other) = delete;
	BufferBase(BufferBase&& other) noexcept;
	
	virtual ~BufferBase() = 0;
	
	GLuint getHandle() const;
	
protected:
	GLuint _handle = -1;
};

