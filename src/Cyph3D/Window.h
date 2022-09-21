#pragma once

#include "Cyph3D/Event.h"

#include <glm/ext.hpp>

struct GLFWwindow;

class Window
{
public:
	explicit Window();
	~Window();
	
	void setCallbacks();
	
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
	
	Event<glm::ivec2>& resizeEvent();
	
	GLFWwindow* getHandle();

private:
	GLFWwindow* _glfwWindow;
	
	Event<glm::ivec2> _resizeEvent;
};