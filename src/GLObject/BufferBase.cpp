#include "BufferBase.h"

GLuint BufferBase::getHandle() const
{
	return _handle;
}

BufferBase::~BufferBase() = default;

BufferBase::BufferBase(BufferBase&& other) noexcept:
_handle(other._handle)
{
	other._handle = -1;
}