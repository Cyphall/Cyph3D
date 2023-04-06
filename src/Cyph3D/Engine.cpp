#include "Engine.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/GlfwHelper.h"
#include "Cyph3D/Helper/RenderHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/UIHelper.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Helper/ThreadHelper.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <format>
#include <stdexcept>

std::unique_ptr<Window> Engine::_window;
std::unique_ptr<AssetManager> Engine::_assetManager;
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
	glfwInit();

	glfwSetErrorCallback([](int code, const char* message) {
		Logger::error(message, "GLFW");
	});

	GlfwHelper::ensureGpuIsCompatible();
	
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
	
//	Logger::SetLogLevel(Logger::LogLevel::Warning);
	
	_window = std::make_unique<Window>();
	
	_assetManager = std::make_unique<AssetManager>(std::max(ThreadHelper::getPhysicalCoreCount() - 1, 1));
	
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
	
	MaterialAsset::initDefaultAndMissing();
	RenderHelper::initDrawScreenQuad();
	Entity::initComponentFactories();
	ShapeRenderer::initShapeFactories();
	UIViewport::initSceneRendererFactories();
	
	_scene = std::make_unique<Scene>();
	
	UIHelper::init();
}

void Engine::run()
{
	while (!_window->shouldClose())
	{
		_timer.onNewFrame();

		glfwPollEvents();

		UIHelper::onNewFrame();

		_assetManager->onUpdate();
		_scene->onUpdate();
		
		UIHelper::render();
		
		_window->swapBuffers();
	}
}

void Engine::shutdown()
{
	UIHelper::shutdown();
	_scene.reset();
	_assetManager.reset();
	_window.reset();
	glfwTerminate();
}

Window& Engine::getWindow()
{
	return *_window;
}

AssetManager& Engine::getAssetManager()
{
	return *_assetManager;
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