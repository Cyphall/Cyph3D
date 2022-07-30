#include "Cyph3D/Helper/ShaderHelper.h"
#include <stdexcept>
#include <sstream>
#include <glad/glad.h>

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