#pragma once

#include <stdexcept>

class OpenGLException : public std::runtime_error
{
public:
	OpenGLException(const char* message) : runtime_error(message){};
	OpenGLException(const std::string& message) : runtime_error(message){};
};

