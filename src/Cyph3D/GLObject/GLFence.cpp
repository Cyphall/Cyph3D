#include "GLFence.h"

GLFence::GLFence()
{
	_handle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

GLFence::~GLFence()
{
	glDeleteSync(_handle);
}

GLsync GLFence::getHandle() const
{
	return _handle;
}

bool GLFence::isSignaled(bool flush)
{
	return waitSignaling(0, flush);
}

bool GLFence::waitSignaling(GLuint64 timeout, bool flush)
{
	GLenum state = glClientWaitSync(_handle, flush ? GL_SYNC_FLUSH_COMMANDS_BIT : 0, timeout);
	return state == GL_ALREADY_SIGNALED || state == GL_CONDITION_SATISFIED;
}