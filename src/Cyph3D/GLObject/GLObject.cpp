#include "GLObject.h"

#include <cassert>

GLObject::~GLObject()
{
	assert(_handle == 0 && "A GL object was not corrently destroyed.");
}

GLuint GLObject::getHandle() const
{
	return _handle;
}