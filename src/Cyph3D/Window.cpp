#include "Window.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"

#include <GLFW/glfw3.h>

Window::Window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	
	_glfwWindow = glfwCreateWindow(800, 600, "Cyph3D", nullptr, nullptr);
	glfwSetInputMode(_glfwWindow, GLFW_RAW_MOUSE_MOTION, true);
	
	VKContext& vkContext = Engine::getVKContext();
	
	VkSurfaceKHR surface;
	glfwCreateWindowSurface(vkContext.getInstance(), _glfwWindow, nullptr, &surface);
	_surface = surface;
	
	_swapchain = VKSwapchain::create(vkContext, _surface);
	
	_previousFrameMouseButtonsPressed.fill(false);
	_currentFrameMouseButtonsPressed.fill(false);
}

Window::~Window()
{
	_swapchain.reset();
	Engine::getVKContext().getInstance().destroySurfaceKHR(_surface);
	glfwDestroyWindow(_glfwWindow);
}

glm::uvec2 Window::getSize()
{
	glm::ivec2 size;
	glfwGetWindowSize(_glfwWindow, &size.x, &size.y);
	return size;
}

glm::uvec2 Window::getSurfaceSize()
{
	glm::ivec2 size;
	glfwGetFramebufferSize(_glfwWindow, &size.x, &size.y);
	return size;
}

float Window::getPixelScale() const
{
	float pixelScale;
	glfwGetWindowContentScale(_glfwWindow, nullptr, &pixelScale);
	return pixelScale;
}

glm::vec2 Window::getCursorPos() const
{
	glm::dvec2 pos;
	glfwGetCursorPos(_glfwWindow, &pos.x, &pos.y);
	return pos;
}

void Window::setCursorPos(const glm::vec2& pos)
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

int Window::getInputMode() const
{
	return glfwGetInputMode(_glfwWindow, GLFW_CURSOR);
}

void Window::setInputMode(int inputMode)
{
	glfwSetInputMode(_glfwWindow, GLFW_CURSOR, inputMode);
}

int Window::getKey(int key)
{
	return glfwGetKey(_glfwWindow, key);
}

Window::MouseButtonState Window::getMouseButtonState(int button)
{
	int previousState = _previousFrameMouseButtonsPressed[button];
	int currentState = _currentFrameMouseButtonsPressed[button];
	
	if (previousState == GLFW_RELEASE && currentState == GLFW_PRESS)
	{
		return MouseButtonState::Clicked;
	}
	if (previousState == GLFW_PRESS && currentState == GLFW_PRESS)
	{
		return MouseButtonState::Held;
	}
	if (previousState == GLFW_PRESS && currentState == GLFW_RELEASE)
	{
		return MouseButtonState::Released;
	}
	if (previousState == GLFW_RELEASE && currentState == GLFW_RELEASE)
	{
		return MouseButtonState::None;
	}
	
	throw;
}

GLFWwindow* Window::getHandle()
{
	return _glfwWindow;
}

void Window::onPollEvents()
{
	_previousFrameMouseButtonsPressed = _currentFrameMouseButtonsPressed;
	for (int i = 0; i < _currentFrameMouseButtonsPressed.size(); i++)
	{
		_currentFrameMouseButtonsPressed[i] = glfwGetMouseButton(_glfwWindow, i) == GLFW_PRESS;
	}
}

VKSwapchain& Window::getSwapchain()
{
	return *_swapchain;
}

void Window::recreateSwapchain()
{
	_swapchain = VKSwapchain::create(Engine::getVKContext(), _surface, _swapchain.get());
}