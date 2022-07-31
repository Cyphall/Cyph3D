#include "Cyph3D/Window.h"
#include <GLFW/glfw3.h>

static void windowResizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
	Window& window = *(Window*)glfwGetWindowUserPointer(glfwWindow);
	window.resizeEvent().invoke(glm::ivec2(width, height));
}

Window::Window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
	
	_glfwWindow = glfwCreateWindow(800, 600, "Cyph3D", nullptr, nullptr);
	
	glfwSetWindowUserPointer(_glfwWindow, this);
	
	glfwMakeContextCurrent(_glfwWindow);
	
	glfwSetInputMode(_glfwWindow, GLFW_RAW_MOUSE_MOTION, true);
	
	setCallbacks();
}

Window::~Window()
{
	glfwDestroyWindow(_glfwWindow);
}

void Window::setCallbacks()
{
	glfwSetWindowSizeCallback(_glfwWindow, windowResizeCallback);
}

glm::ivec2 Window::getSize()
{
	glm::ivec2 size;
	glfwGetWindowSize(_glfwWindow, &size.x, &size.y);
	return size;
}

glm::dvec2 Window::getCursorPos() const
{
	glm::dvec2 pos;
	glfwGetCursorPos(_glfwWindow, &pos.x, &pos.y);
	return pos;
}

void Window::setCursorPos(const glm::dvec2& pos)
{
	glfwSetCursorPos(_glfwWindow, pos.x, pos.y);
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(_glfwWindow);
}

void Window::setShouldClose(bool value)
{
	glfwSetWindowShouldClose(_glfwWindow, value);
}

int Window::getKey(int key)
{
	return glfwGetKey(_glfwWindow, key);
}

int Window::getMouseButton(int button)
{
	return glfwGetMouseButton(_glfwWindow, button);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(_glfwWindow);
}

GLFWwindow* Window::getHandle()
{
	return _glfwWindow;
}

Event<glm::ivec2>& Window::resizeEvent()
{
	return _resizeEvent;
}

int Window::getInputMode() const
{
	return glfwGetInputMode(_glfwWindow, GLFW_CURSOR);
}

void Window::setInputMode(int inputMode)
{
	glfwSetInputMode(_glfwWindow, GLFW_CURSOR, inputMode);
}