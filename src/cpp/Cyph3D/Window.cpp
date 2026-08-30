#include "Window.h"

#include <Cyph3D/Engine.h>

#include <backends/imgui_impl_glfw.h>
#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/Surface.hpp>
#include <CyphGPU/Swapchain.hpp>
#include <GLFW/glfw3.h>
#include <thread>

c3d::Window::Window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	_glfwWindow = glfwCreateWindow(800, 600, "Cyph3D", nullptr, nullptr);

	// WORKAROUND: X11 (or XWayland) is buggy and will not report the right window
	// extent until after some time has passed
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	glfwSetInputMode(_glfwWindow, GLFW_RAW_MOUSE_MOTION, true);

	VkSurfaceKHR surfaceRaw{};
	vk::detail::resultCheck(
		static_cast<vk::Result>(
			glfwCreateWindowSurface(Engine::getDeviceSession()->getDevice()->getContextSession()->getHandle(), _glfwWindow, nullptr, &surfaceRaw)
		),
		"glfwCreateWindowSurface"
	);

	_surface = cgpu::Surface::create(
		Engine::getDeviceSession()->getDevice()->getContextSession(),
		{
			.surface = surfaceRaw,
		}
	);

	constexpr std::array<vk::SurfaceFormatKHR, 2> preferredSurfaceFormats = {
		vk::SurfaceFormatKHR{
			.format = vk::Format::eA2B10G10R10UnormPack32,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		},
		vk::SurfaceFormatKHR{
			.format = vk::Format::eB8G8R8A8Unorm,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		}
	};

	auto surfaceFormat = Engine::getDeviceSession()->getDevice()->selectBestSurfaceFormat(_surface, preferredSurfaceFormats);
	if (!surfaceFormat)
	{
		throw std::runtime_error("None of the requested surface formats are available.");
	}

	_surfaceFormat = *surfaceFormat;

	ensureValidSwapchain();

	_previousFrameMouseButtonsPressed.fill(false);
	_currentFrameMouseButtonsPressed.fill(false);
}

c3d::Window::~Window()
{
	Engine::getDeviceSession()->waitIdle();
	_swapchain.reset();
	_surface.reset();
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
	return ImGui_ImplGlfw_GetContentScaleForWindow(_glfwWindow);
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
		return MouseButtonState::eClicked;
	}
	if (previousState == GLFW_PRESS && currentState == GLFW_PRESS)
	{
		return MouseButtonState::eHeld;
	}
	if (previousState == GLFW_PRESS && currentState == GLFW_RELEASE)
	{
		return MouseButtonState::eReleased;
	}
	if (previousState == GLFW_RELEASE && currentState == GLFW_RELEASE)
	{
		return MouseButtonState::eNone;
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

const vk::SurfaceFormatKHR& c3d::Window::getSurfaceFormat()
{
	return _surfaceFormat;
}

const cgpu::SwapchainPtr& c3d::Window::getSwapchain()
{
	return _swapchain;
}

void c3d::Window::ensureValidSwapchain()
{
	while (!_swapchain || !_swapchain->tryGetImage())
	{
		glm::ivec2 extent;
		glfwGetFramebufferSize(_glfwWindow, &extent.x, &extent.y);
		if (extent.x == 0 || extent.y == 0)
		{
			glfwWaitEvents();
			continue;
		}

		_swapchain = cgpu::Swapchain::create(
			Engine::getDeviceSession(),
			_surface,
			{
				.format = _surfaceFormat,
				.preferred_extent = extent,
				.usages = vk::ImageUsageFlagBits::eColorAttachment,
				.old_swapchain = _swapchain ? std::optional{_swapchain} : std::nullopt,
			}
		);
	}
}
