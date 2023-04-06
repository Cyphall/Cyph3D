#pragma once

#include <glm/glm.hpp>
#include <array>

struct GLFWwindow;

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
	
	glm::ivec2 getSize();
	
	float getPixelScale() const;
	
	glm::vec2 getCursorPos() const;
	void setCursorPos(const glm::vec2& pos);
	
	bool shouldClose() const;
	void setShouldClose(bool value);
	
	void swapBuffers();
	
	int getInputMode() const;
	void setInputMode(int inputMode);
	
	int getKey(int key);
	
	MouseButtonState getMouseButtonState(int button);
	
	GLFWwindow* getHandle();
	
	void onPollEvents();

private:
	GLFWwindow* _glfwWindow;
	
	std::array<bool, 8> _previousFrameMouseButtonsPressed;
	std::array<bool, 8> _currentFrameMouseButtonsPressed;
};