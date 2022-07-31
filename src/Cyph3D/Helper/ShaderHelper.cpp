#include "ShaderHelper.h"

#include <glad/glad.h>
#include <sstream>
#include <stdexcept>

std::string ShaderHelper::shaderTypeToExtension(GLenum type)
{
	switch (type)
	{
		case GL_VERTEX_SHADER:
			return "vert";
		case GL_FRAGMENT_SHADER:
			return "frag";
		case GL_GEOMETRY_SHADER:
			return "geom";
		case GL_COMPUTE_SHADER:
			return "comp";
		default:
			std::stringstream error;
			error << "Shader type " << type << " is not currently supported";
			throw std::runtime_error(error.str());
	}
}