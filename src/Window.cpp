#include "Window.h"

void KeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, uint32_t mods)
{
	Window& window = *(Window*)glfwGetWindowUserPointer(glfwWindow);
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				window.setGuiOpen(!window.isGuiOpen());
				break;
			case GLFW_KEY_X:
				if (mods & GLFW_MOD_CONTROL)
					window.setShouldClose(true);
				break;
		}
	}
}

Window::Window(std::optional<glm::ivec2> size):
_guiOpen(false)
{
	glfwInit();
	
	glfwSetErrorCallback(
			[](int code, const char *message)
			{
				Logger::Error(message, "GLFW");
			}
	);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
	
	if (size.has_value())
	{
		_glfwWindow = glfwCreateWindow(size->x, size->y, "Cyph3D", nullptr, nullptr);
		const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (vidmode != nullptr)
		{
			glfwSetWindowPos(_glfwWindow, (vidmode->width - size->x) / 2, (vidmode->height- size->y) / 2);
		}
	}
	else
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		_glfwWindow = glfwCreateWindow(mode->width, mode->height, "Cyph3D", monitor, nullptr);
	}
	
	glfwSetWindowUserPointer(_glfwWindow, this);
	
	glfwMakeContextCurrent(_glfwWindow);
	setGuiOpen(false);
	
	glfwSetInputMode(_glfwWindow, GLFW_RAW_MOUSE_MOTION, true);
	
	setCallbacks();
}

void Window::setCallbacks()
{
	glfwSetKeyCallback(_glfwWindow, reinterpret_cast<GLFWkeyfun>(KeyCallback));
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

bool Window::isGuiOpen() const
{
	return _guiOpen;
}

void Window::setGuiOpen(bool value)
{
	_guiOpen = value;
	
	if (value)
	{
		glfwSetInputMode(_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		setCursorPos(getSize() / 2);
	}
	else
	{
		glfwSetInputMode(_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
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
