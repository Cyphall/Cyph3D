#include "Engine.h"
#include "Helper/GlfwHelper.h"
#include <stdexcept>
#include <stb_image.h>
#include <windows.h>
#include "UI/UIHelper.h"
#include "GLObject/Framebuffer.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include "Window.h"
#include "ResourceManagement/ResourceManager.h"
#include "Scene/Scene.h"
#include "Rendering/Renderer.h"
#include "Helper/RenderHelper.h"
#include <fmt/core.h>
#include <thread_pool.hpp>

std::unique_ptr<Window> Engine::_window;
std::unique_ptr<ResourceManager> Engine::_globalResourceManager;
std::unique_ptr<Scene> Engine::_scene;
std::unique_ptr<Renderer> Engine::_renderer;

double Engine::_previousTime = 0;

void messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			Logger::Error(message, "OPGL");
			if (IsDebuggerPresent())
				__debugbreak();
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			Logger::Warning(message, "OPGL");
			break;
		case GL_DEBUG_SEVERITY_LOW:
			Logger::Info(message, "OPGL");
			break;
	}
}

void Engine::init()
{
	GlfwHelper::Init();
	
	stbi_set_flip_vertically_on_load(true);
	
//	Logger::SetLogLevel(Logger::LogLevel::Warning);
	
//	_window = std::make_unique<Window>(glm::ivec2(1600, 900));
	_window = std::make_unique<Window>();
	_globalResourceManager = std::make_unique<ResourceManager>(1);
	
	Logger::Info(fmt::format("GLFW Version: {}", glfwGetVersionString()), "GLFW");
	Logger::Info(fmt::format("OpenGL Version: {}", glGetString(GL_VERSION)), "OPGL");
	Logger::Info(fmt::format("GPU: {}", glGetString(GL_RENDERER)), "OPGL");
	
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
	glDebugMessageCallback(messageCallback, nullptr);
	
	Material::initializeDefault();
	Framebuffer::initDrawToDefault();
	RenderHelper::initDrawScreenQuad();
	
	_renderer = std::make_unique<Renderer>();
	
	_scene = std::make_unique<Scene>();
	
	UIHelper::init();
}

void Engine::run()
{
	while (!_window->shouldClose())
	{
		glfwPollEvents();
		
		_globalResourceManager->update();
		_scene->getRM().update();
		
		double currentTime = glfwGetTime();
		_scene->update(currentTime - _previousTime);
		_previousTime = currentTime;
		
		_renderer->render();
		
		UIHelper::update();
		UIHelper::render();
		
		_window->swapBuffers();
	}
}

void Engine::shutdown()
{
	UIHelper::shutdown();
	glfwTerminate();
}

Window& Engine::getWindow()
{
	return *_window;
}

ResourceManager& Engine::getGlobalRM()
{
	return *_globalResourceManager;
}

Scene& Engine::getScene()
{
	return *_scene;
}

void Engine::setScene(std::unique_ptr<Scene>&& scene)
{
	_scene = std::move(scene);
}

Renderer& Engine::getRenderer()
{
	return *_renderer;
}
