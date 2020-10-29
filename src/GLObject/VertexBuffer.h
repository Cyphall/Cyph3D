#pragma once

#include "Buffer.h"

template<typename T>
class VertexBuffer : public Buffer<T>
{
public:
	explicit VertexBuffer(GLenum usage, int elementCountPerVertex):
	Buffer<T>(usage), _elementCountPerVertex(elementCountPerVertex)
	{
	
	}
	
	VertexBuffer(GLsizeiptr count, GLenum flags, int elementCountPerVertex):
	Buffer<T>(count, flags), _elementCountPerVertex(elementCountPerVertex)
	{
	
	}
	
	int getElementCountPerVertex() const
	{
		return _elementCountPerVertex;
	}

private:
	int _elementCountPerVertex;
};
