#pragma once

#include <stack>
#include <glad/glad.h>
#include <functional>

class GLStateManager
{
public:
	static void push();
	static void pop();
	
	static void setDepthTest(bool value);
	static void setCullFace(bool value);
	static void setBlend(bool value);
	static void setDepthMask(bool value);
	
	static void setDepthFunc(GLenum value);
	static void setFrontFace(GLenum value);
	static void setBlendFunc(GLenum sFactor, GLenum dFactor);
	static void setBlendEquation(GLenum value);
	
	static void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);
	static void setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	
private:
	static std::stack<std::stack<std::function<void()>>> _restoreStateActions;
};


