#pragma once

#include "Cyph3D/GLObject/GLObject.h"

#include <string>

class GLShader : public GLObject
{
public:
	GLShader(GLenum type, const std::string& file);
	~GLShader() override;
};