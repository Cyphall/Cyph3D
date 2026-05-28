#include "Window.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"

#include <GLFW/glfw3.h>

c3d::Window::Window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	_glfwWindow = glfwCreateWindow(800, 600, "Cyph3D", nullptr, nullptr);
	glfwSetInputMode(_glfwWindow, GLFW_RAW_MOUSE_MOTION, true);

	VKContext& vkContext = Engine::getVKContext();

	VkSurfaceKHR surface;
	glfwCreateWindowSurface(vkContext.getInstance(), _glfwWindow, nullptr, &surface);
	_surface = surface;

	glm::ivec2 extent;
	glfwGetFramebufferSize(_glfwWindow, &extent.x, &extent.y);
	_swapchain = VKSwapchain::create(vkContext, _surface, extent);

	_previousFrameMouseButtonsPressed.fill(false);
	_currentFrameMouseButtonsPressed.fill(false);
}

c3d::Window::~Window()
{
	_swapchain.reset();
	Engine::getVKContext().getInstance().destroySurfaceKHR(_surface);
	glfwDestroyWindow(_glfwWindow);
}

glm::uvec2 c3d::Window::getSize()
{
	glm::ivec2 size;
	glfwGetWindowSize(_glfwWindow, &size.x, &size.y);
	return size;
}

glm::uvec2 c3d::Window::getSurfaceSize()
{
	glm::ivec2 size;
	glfwGetFramebufferSize(_glfwWindow, &size.x, &size.y);
	return size;
}

float c3d::Window::getPixelScale() const
{
	float pixelScale;
	glfwGetWindowContentScale(_glfwWindow, nullptr, &pixelScale);
	return pixelScale;
}

glm::vec2 c3d::Window::getCursorPos() const
{
	glm::dvec2 pos;
	glfwGetCursorPos(_glfwWindow, &pos.x, &pos.y);
	return pos;
}

void c3d::Window::setCursorPos(const glm::vec2& pos)
{
	glfwSetCursorPos(_glfwWindow, pos.x, pos.y);
}

bool c3d::Window::shouldClose() const
{
	return glfwWindowShouldClose(_glfwWindow);
}

void c3d::Window::setShouldClose(bool value)
{
	glfwSetWindowShouldClose(_glfwWindow, value);
}

int c3d::Window::getInputMode() const
{
	return glfwGetInputMode(_glfwWindow, GLFW_CURSOR);
}

void c3d::Window::setInputMode(int inputMode)
{
	glfwSetInputMode(_glfwWindow, GLFW_CURSOR, inputMode);
}

int c3d::Window::getKey(int key)
{
	return glfwGetKey(_glfwWindow, key);
}

c3d::Window::MouseButtonState c3d::Window::getMouseButtonState(int button)
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

GLFWwindow* c3d::Window::getHandle()
{
	return _glfwWindow;
}

void c3d::Window::onPollEvents()
{
	_previousFrameMouseButtonsPressed = _currentFrameMouseButtonsPressed;
	for (int i = 0; i < _currentFrameMouseButtonsPressed.size(); i++)
	{
		_currentFrameMouseButtonsPressed[i] = glfwGetMouseButton(_glfwWindow, i) == GLFW_PRESS;
	}
}

c3d::VKSwapchain& c3d::Window::getSwapchain()
{
	return *_swapchain;
}

void c3d::Window::recreateSwapchain()
{
	glm::ivec2 extent;
	glfwGetFramebufferSize(_glfwWindow, &extent.x, &extent.y);
	_swapchain = VKSwapchain::create(Engine::getVKContext(), _surface, extent, _swapchain.get());
}