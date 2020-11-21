#include "GLStateManager.h"

GLboolean GLStateManager::_defaultDepthTest;
GLboolean GLStateManager::_defaultCullFace;
GLboolean GLStateManager::_defaultBlend;
GLboolean GLStateManager::_defaultDepthMask;

GLenum GLStateManager::_defaultDepthFunc;
GLenum GLStateManager::_defaultFrontFace;
GLenum GLStateManager::_defaultBlendFunc[2];
GLenum GLStateManager::_defaultBlendEquation[2];

GLint GLStateManager::_defaultViewport[4];
GLfloat GLStateManager::_defaultClearColor[4];

void GLStateManager::initialize()
{
	_defaultDepthTest = glIsEnabled(GL_DEPTH_TEST);
	_defaultCullFace = glIsEnabled(GL_CULL_FACE);
	_defaultBlend = glIsEnabled(GL_BLEND);
	
	glGetBooleanv(GL_COLOR_WRITEMASK, _defaultColorMask);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &_defaultDepthMask);
	glGetIntegerv(GL_STENCIL_WRITEMASK, &_defaultStencilMask);
	
	GLint defaultDepthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &defaultDepthFunc);
	_defaultDepthFunc = defaultDepthFunc;
	
	GLint defaultFrontFace;
	glGetIntegerv(GL_FRONT_FACE, &defaultFrontFace);
	_defaultFrontFace = defaultFrontFace;
	
	glGetFloatv(GL_COLOR_CLEAR_VALUE, _defaultClearColor);
	
	GLint defaultBlendFunc[2];
	glGetIntegerv(GL_BLEND_SRC, &defaultBlendFunc[0]);
	glGetIntegerv(GL_BLEND_DST, &defaultBlendFunc[1]);
	_defaultBlendFunc[0] = defaultBlendFunc[0];
	_defaultBlendFunc[1] = defaultBlendFunc[1];
	
	GLint defaultBlendEquation[2];
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &defaultBlendEquation[0]);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &defaultBlendEquation[1]);
	_defaultBlendEquation[0] = defaultBlendEquation[0];
	_defaultBlendEquation[1] = defaultBlendEquation[1];
	
	glGetIntegerv(GL_VIEWPORT, _defaultViewport);
}

void GLStateManager::use(const GLPipelineState& state)
{
	if (state.depthTest ? state.depthTest.value() : _defaultDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	
	if (state.cullFace ? state.cullFace.value() : _defaultCullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	
	if (state.blend ? state.blend.value() : _defaultBlend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	
	if (state.colorMask)
	{
		glColorMask(state.colorMask.value()[0], state.colorMask.value()[1], state.colorMask.value()[2], state.colorMask.value()[3]);
	}
	else
	{
		glColorMask(_defaultColorMask[0], _defaultColorMask[1], _defaultColorMask[2], _defaultColorMask[3]);
	}
	
	glDepthMask(state.depthMask ? state.depthMask.value() : _defaultDepthMask);
	
	glStencilMask(state.stencilMask ? state.stencilMask.value() : _defaultStencilMask);
	
	glDepthFunc(state.depthFunc ? state.depthFunc.value() : _defaultDepthFunc);
	
	glFrontFace(state.frontFace ? state.frontFace.value() : _defaultFrontFace);
	
	if (state.clearColor)
	{
		glClearColor(state.clearColor.value()[0], state.clearColor.value()[1], state.clearColor.value()[2], state.clearColor.value()[3]);
	}
	else
	{
		glClearColor(_defaultClearColor[0], _defaultClearColor[1], _defaultClearColor[2], _defaultClearColor[3]);
	}
	
	if (state.blendFunc)
	{
		glBlendFunc(state.blendFunc.value()[0], state.blendFunc.value()[1]);
	}
	else
	{
		glBlendFunc(_defaultBlendFunc[0], _defaultBlendFunc[1]);
	}
	
	if (state.blendEquation)
	{
		glBlendEquation(state.blendEquation.value());
	}
	else
	{
		glBlendEquationSeparate(_defaultBlendEquation[0], _defaultBlendEquation[1]);
	}
	
	if (state.viewport)
	{
		glViewport(state.viewport.value()[0], state.viewport.value()[1], state.viewport.value()[2], state.viewport.value()[3]);
	}
	else
	{
		glViewport(_defaultViewport[0], _defaultViewport[1], _defaultViewport[2], _defaultViewport[3]);
	}
	
}
