#pragma once

#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <optional>

class Window
{
public:
	explicit Window(std::optional<glm::ivec2> size = std::nullopt);
	void setCallbacks();
	
	glm::ivec2 getSize();
	
	glm::dvec2 getCursorPos() const;
	void setCursorPos(const glm::dvec2& pos);
	
	bool shouldClose() const;
	void setShouldClose(bool value);
	
	bool isGuiOpen() const;
	void setGuiOpen(bool value);
	
	void swapBuffers();
	
	int getKey(int key);
	int getMouseButton(int button);
	
	GLFWwindow* getHandle();

private:
	GLFWwindow* _glfwWindow;
	bool _guiOpen;
};
