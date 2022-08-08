#pragma once

#include <stdexcept>

class OpenGLException : public std::runtime_error
{
public:
	explicit OpenGLException(const char* message) : runtime_error(message){};
	explicit OpenGLException(const std::string& message) : runtime_error(message){};
};