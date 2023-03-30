#pragma once

#include <glm/ext.hpp>

struct GLFWwindow;

class Window
{
public:
	explicit Window();
	~Window();
	
	glm::ivec2 getSize();
	
	float getPixelScale() const;
	
	glm::dvec2 getCursorPos() const;
	void setCursorPos(const glm::dvec2& pos);
	
	bool shouldClose() const;
	void setShouldClose(bool value);
	
	void swapBuffers();
	
	int getInputMode() const;
	void setInputMode(int inputMode);
	
	int getKey(int key);
	int getMouseButton(int button);
	
	GLFWwindow* getHandle();

private:
	GLFWwindow* _glfwWindow;
};