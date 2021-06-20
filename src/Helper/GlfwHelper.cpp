#include "GlfwHelper.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../Logger.h"
#include "../Exception/OpenGLException.h"
#include <sstream>
#include <windows.h>

#define VERSION(major, minor, revision) (major * 100 + minor * 10 + revision)

void GlfwHelper::Init()
{
	glfwInit();
	
	glfwSetErrorCallback([](int code, const char* message) {
		Logger::Error(message, "GLFW");
		if (IsDebuggerPresent())
			__debugbreak();
	});
	
	EnsureGpuIsCompatible();
}

void GlfwHelper::EnsureGpuIsCompatible()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, false);
	
	GLFWwindow* window = glfwCreateWindow(1, 1, "CompatibilityQuery", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
	
	int major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	int minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	int revision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	int maxSupportedOpenGLVersion = VERSION(major, minor, revision);
	int requestedOpenGLVersion = VERSION(4, 6, 0);
	
	std::stringstream errorMessage;
	bool error = false;
	if (maxSupportedOpenGLVersion < requestedOpenGLVersion)
	{
		errorMessage << "OpenGL " << requestedOpenGLVersion << " is not supported by this driver.\n";
		errorMessage << "Please make sure your GPU is compatible and your driver is up to date.\n\n";
		errorMessage << "Driver: " << glGetString(GL_VERSION) << "\n";
		errorMessage << "GPU: " << glGetString(GL_RENDERER) << "\n";
		error = true;
	}
	
	std::string requiredExtensions[] = {
			"GL_ARB_bindless_texture"
	};
	
	for (const std::string& extension : requiredExtensions)
	{
		if (error)
			break;
		
		if (!glfwExtensionSupported(extension.c_str()))
		{
			errorMessage << "OpenGL extension " << extension << " is not supported by this driver.\n";
			errorMessage << "Please make sure your GPU is compatible and your driver is up to date.\n\n";
			errorMessage << "Driver: " << glGetString(GL_VERSION) << "\n";
			errorMessage << "GPU: " << glGetString(GL_RENDERER) << "\n";
			error = true;
		}
	}
	
	glfwMakeContextCurrent(nullptr);
	glfwDestroyWindow(window);
	
	glfwWindowHint(GLFW_VISIBLE, true);
	
	if (error)
	{
		throw OpenGLException(errorMessage.str());
	}
}
