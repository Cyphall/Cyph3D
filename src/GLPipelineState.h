#pragma once

#include <glad/glad.h>
#include <optional>
#include <array>

struct GLPipelineState
{
	std::optional<bool> depthTest;
	std::optional<GLenum> depthFunc;
	
	std::optional<std::array<bool, 4>> colorMask;
	std::optional<bool> depthMask;
	std::optional<int> stencilMask;
	
	std::optional<bool> cullFace;
	std::optional<GLenum> frontFace;
	
	std::optional<bool> blend;
	std::optional<std::array<GLenum, 2>> blendFunc;
	std::optional<GLenum> blendEquation;
	
	std::optional<std::array<GLint, 4>> viewport;
	
	std::optional<std::array<GLfloat, 4>> clearColor;
};
