#pragma once

#include <array>
#include <CyphGPU/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace c3d
{
class Window
{
public:
	enum class MouseButtonState
	{
		eClicked,
		eHeld,
		eReleased,
		eNone
	};

	explicit Window();
	~Window();

	glm::uvec2 getSize();

	glm::uvec2 getSurfaceSize();

	float getPixelScale() const;

	glm::vec2 getCursorPos() const;
	void setCursorPos(const glm::vec2& pos);

	bool shouldClose() const;
	void setShouldClose(bool value);

	int getInputMode() const;
	void setInputMode(int inputMode);

	int getKey(int key);

	MouseButtonState getMouseButtonState(int button);

	GLFWwindow* getHandle();

	void onPollEvents();

	const vk::SurfaceFormatKHR& getSurfaceFormat();

	const cgpu::SwapchainPtr& getSwapchain();

	void ensureValidSwapchain();

private:
	GLFWwindow* _glfwWindow;

	std::array<bool, 8> _previousFrameMouseButtonsPressed;
	std::array<bool, 8> _currentFrameMouseButtonsPressed;

	vk::SurfaceFormatKHR _surfaceFormat;
	cgpu::SurfacePtr _surface;
	cgpu::SwapchainPtr _swapchain;
};
}
