#pragma once

#include <glad/glad.h>

class GLObject
{
public:
	GLObject() = default;
	virtual ~GLObject() = 0;
	
	GLObject(const GLObject& other) = delete;
	GLObject& operator=(const GLObject& other) = delete;
	
	GLObject(GLObject&& other) = delete;
	GLObject& operator=(GLObject&& other) = delete;
	
	GLuint getHandle() const;

protected:
	GLuint _handle = 0;
};