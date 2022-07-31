#include "GLVertexArray.h"
#include "Cyph3D/GLObject/GLBuffer.h"

GLVertexArray::GLVertexArray()
{
	glCreateVertexArrays(1, &_handle);
}

GLVertexArray::~GLVertexArray()
{
	glDeleteVertexArrays(1, &_handle);
	_handle = 0;
}

void GLVertexArray::defineFormat(int bufferSlot, int attribLocation, int elementCount, GLenum elementType, int offset)
{
	glVertexArrayAttribBinding(_handle, attribLocation, bufferSlot); // sets which buffer slot will be used for this vertex attribute
	glEnableVertexArrayAttrib(_handle, attribLocation); // enables this vertex attribute
	glVertexArrayAttribFormat(_handle, attribLocation, elementCount, elementType, false, offset); // defines this vertex attribute
}

void GLVertexArray::bindIndexBuffer(const GLBuffer<GLuint>& buffer)
{
	glVertexArrayElementBuffer(_handle, buffer.getHandle());
}

void GLVertexArray::bind()
{
	glBindVertexArray(_handle);
}