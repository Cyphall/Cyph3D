#include "Engine.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/Helper/GlfwHelper.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/UIHelper.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Window.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <format>
#include <stdexcept>

std::unique_ptr<Window> Engine::_window;
std::unique_ptr<ResourceManager> Engine::_globalResourceManager;
std::unique_ptr<Scene> Engine::_scene;

Timer Engine::_timer;

static void messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			Logger::error(message, "OPGL");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			Logger::warning(message, "OPGL");
			break;
		case GL_DEBUG_SEVERITY_LOW:
			Logger::info(message, "OPGL");
			break;
	}
}

void Engine::init()
{
	GlfwHelper::Init();
	
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
	
//	Logger::SetLogLevel(Logger::LogLevel::Warning);
	
	_window = std::make_unique<Window>();
	
	_globalResourceManager = std::make_unique<ResourceManager>(1);
	
	Logger::info(std::format("GLFW Version: {}", glfwGetVersionString()), "GLFW");
	Logger::info(std::format("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION))), "OPGL");
	Logger::info(std::format("GPU: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER))), "OPGL");
	
#if defined(_DEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
//	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
	glDebugMessageCallback(messageCallback, nullptr);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	Material::initialize();
	RenderHelper::initDrawScreenQuad();
	Entity::initAllocators();
	ShapeRenderer::initAllocators();
	UIViewport::initAllocators();
	
	_scene = std::make_unique<Scene>();
	
	UIHelper::init();
}

void Engine::run()
{
	while (!_window->shouldClose())
	{
		glfwPollEvents();
		
		_timer.onNewFrame();
		UIHelper::onNewFrame();
		
		_globalResourceManager->onUpdate();
		_scene->onUpdate();
		
		UIHelper::render();
		
		_window->swapBuffers();
	}
}

void Engine::shutdown()
{
	UIHelper::shutdown();
	_scene.reset();
	_globalResourceManager.reset();
	_window.reset();
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
	UIInspector::setSelected(nullptr);
	_scene = std::move(scene);
}

Timer& Engine::getTimer()
{
	return _timer;
}