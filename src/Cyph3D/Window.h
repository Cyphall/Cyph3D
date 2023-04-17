#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;
class VKContext;
class VKSwapchain;

class Window
{
public:
	enum class MouseButtonState
	{
		Clicked,
		Held,
		Released,
		None
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
	
	VKSwapchain& getSwapchain();
	
	void recreateSwapchain();

private:
	GLFWwindow* _glfwWindow;
	
	std::array<bool, 8> _previousFrameMouseButtonsPressed;
	std::array<bool, 8> _currentFrameMouseButtonsPressed;
	
	vk::SurfaceKHR _surface;
	std::unique_ptr<VKSwapchain> _swapchain;
};