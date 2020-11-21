#pragma once

#include <stack>
#include <glad/glad.h>
#include <functional>
#include "GLPipelineState.h"

class GLStateManager
{
public:
	static void initialize();
	
	static void use(const GLPipelineState& state);
	
private:
	static GLboolean _defaultDepthTest;
	static GLboolean _defaultCullFace;
	static GLboolean _defaultBlend;
	
	static GLboolean _defaultColorMask[4];
	static GLboolean _defaultDepthMask;
	static GLuint _defaultStencilMask;
	
	static GLenum _defaultDepthFunc;
	static GLenum _defaultFrontFace;
	static GLenum _defaultBlendFunc[2];
	static GLenum _defaultBlendEquation[2];
	
	static GLint _defaultViewport[4];
	static GLfloat _defaultClearColor[4];
};


