#include "GLStateManager.h"

std::stack<std::stack<std::function<void()>>> GLStateManager::_restoreStateActions;

void GLStateManager::push()
{
	_restoreStateActions.emplace();
}

void GLStateManager::pop()
{
	auto actions = _restoreStateActions.top();
	while (!actions.empty())
	{
		actions.top()();
		actions.pop();
	}
	_restoreStateActions.pop();
}

void GLStateManager::setDepthTest(bool value)
{
	bool oldValue = glIsEnabled(GL_DEPTH_TEST);
	
	if (value)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	
	_restoreStateActions.top().push(
			[=]
			{
				if (oldValue)
					glEnable(GL_DEPTH_TEST);
				else
					glDisable(GL_DEPTH_TEST);
			});
}

void GLStateManager::setCullFace(bool value)
{
	bool oldValue = glIsEnabled(GL_CULL_FACE);
	
	if (value)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	
	_restoreStateActions.top().push(
			[=]
			{
				if (oldValue)
					glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);
			});
}

void GLStateManager::setBlend(bool value)
{
	bool oldValue = glIsEnabled(GL_CULL_FACE);
	
	if (value)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	
	_restoreStateActions.top().push(
			[=]
			{
				if (oldValue)
					glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);
			});
}

void GLStateManager::setDepthMask(bool value)
{
	GLboolean oldValue;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &oldValue);
	
	glDepthMask(value);
	
	_restoreStateActions.top().push(
			[=]
			{
				glDepthMask(oldValue);
			});
}

void GLStateManager::setDepthFunc(GLenum value)
{
	GLint oldValue;
	glGetIntegerv(GL_DEPTH_FUNC, &oldValue);
	
	glDepthFunc(value);
	
	_restoreStateActions.top().push(
			[=]
			{
				glDepthFunc(oldValue);
			});
}

void GLStateManager::setFrontFace(GLenum value)
{
	GLint oldValue;
	glGetIntegerv(GL_FRONT_FACE, &oldValue);
	
	glFrontFace(value);
	
	_restoreStateActions.top().push(
			[=]
			{
				glFrontFace(oldValue);
			});
}

void GLStateManager::setBlendFunc(GLenum sFactor, GLenum dFactor)
{
	GLint oldValue1;
	GLint oldValue2;
	glGetIntegerv(GL_BLEND_SRC, &oldValue1);
	glGetIntegerv(GL_BLEND_DST, &oldValue2);
	
	glBlendFunc(sFactor, dFactor);
	
	_restoreStateActions.top().push(
			[=]
			{
				glBlendFunc(oldValue1, oldValue2);
			});
}

void GLStateManager::setBlendEquation(GLenum value)
{
	GLint oldValue1;
	GLint oldValue2;
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &oldValue1);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &oldValue2);
	
	glBlendEquation(value);
	
	_restoreStateActions.top().push(
			[=]
			{
				glBlendEquationSeparate(oldValue1, oldValue2);
			});
}

void GLStateManager::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	GLint oldValue[4];
	glGetIntegerv(GL_VIEWPORT, oldValue);
	
	glViewport(x, y, width, height);
	
	_restoreStateActions.top().push(
			[=]
			{
				glViewport(oldValue[0], oldValue[1], oldValue[2], oldValue[3]);
			});
}

void GLStateManager::setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	GLint oldValue[4];
	glGetIntegerv(GL_COLOR_CLEAR_VALUE, oldValue);
	
	glClearColor(r, g, b, a);
	
	_restoreStateActions.top().push(
			[=]
			{
				glClearColor(oldValue[0], oldValue[1], oldValue[2], oldValue[3]);
			});
}
