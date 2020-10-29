#include "VertexArray.h"

VertexArray::VertexArray()
{
	glCreateVertexArrays(1, &_handle);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &_handle);
}

void VertexArray::registerIndexBuffer(const BufferBase& buffer)
{
	glVertexArrayElementBuffer(_handle, buffer.getHandle());
}

void VertexArray::bind()
{
	glBindVertexArray(_handle);
}
