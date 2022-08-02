#pragma once

#include <glad/glad.h>

struct SamplerCreateInfo;

class GLFence
{
public:
	GLFence();
	~GLFence();

	GLFence(const GLFence& other) = delete;
	GLFence& operator=(const GLFence& other) = delete;

	GLFence(GLFence&& other) = delete;
	GLFence& operator=(GLFence&& other) = delete;

	GLsync getHandle() const;
	
	bool isSignaled(bool flush = false);
	
	bool waitSignaling(GLuint64 timeout, bool flush = false);
	
private:
	GLsync _handle;
};