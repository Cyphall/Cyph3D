#include "Engine.h"

#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/UIHelper.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Helper/ThreadHelper.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <GLFW/glfw3.h>
#include <format>
#include <stdexcept>

std::unique_ptr<VKContext> Engine::_vkContext;
std::unique_ptr<Window> Engine::_window;
std::unique_ptr<AssetManager> Engine::_assetManager;
std::unique_ptr<Scene> Engine::_scene;

Timer Engine::_timer;

void Engine::init()
{
	glfwInit();

	glfwSetErrorCallback([](int code, const char* message) {
		Logger::error(message, "GLFW");
	});
	
//	Logger::SetLogLevel(Logger::LogLevel::Warning);

	_vkContext = VKContext::create(2);

	Logger::info(std::format("GPU: {}", static_cast<std::string_view>(_vkContext->getPhysicalDevice().getProperties().deviceName)));
	
	_window = std::make_unique<Window>();
	
	_assetManager = std::make_unique<AssetManager>(std::max(ThreadHelper::getPhysicalCoreCount() - 1, 1));
	
	MaterialAsset::initDefaultAndMissing();
	Entity::initComponentFactories();
	
	_scene = std::make_unique<Scene>();
	
	UIHelper::init();
	FileHelper::init();
}

void Engine::run()
{
	bool swapchainOutOfData = true;
	
	while (!_window->shouldClose())
	{
		_vkContext->onNewFrame();
		
		if (swapchainOutOfData)
		{
			glm::uvec2 surfaceSize = _window->getSurfaceSize();
			
			while (surfaceSize.x * surfaceSize.y == 0)
			{
				glfwWaitEvents();
				surfaceSize = _window->getSurfaceSize();
			}
			
			_window->recreateSwapchain();
			
			swapchainOutOfData = false;
		}
		
		uint64_t nextPresentId = _window->getSwapchain().getNextPresentId();
		uint64_t framesToWait = _vkContext->getConcurrentFrameCount() + 1;
		if (nextPresentId > framesToWait)
		{
			uint64_t waitPresentId = nextPresentId - framesToWait;
			_vkContext->getDevice().waitForPresentKHR(_window->getSwapchain().getHandle(), waitPresentId, UINT64_MAX);
		}
		
		VKSwapchain::NextImageInfo nextImageInfo = _window->getSwapchain().retrieveNextImage();
		
		_timer.onNewFrame();

		glfwPollEvents();
		_window->onPollEvents();

		UIHelper::onNewFrame();

		_assetManager->onUpdate();
		_scene->onUpdate();
		
		const VKPtr<VKSemaphore>& renderFinishedSemaphore = UIHelper::render(nextImageInfo.imageView, nextImageInfo.imageAvailableSemaphore);
		if (!_vkContext->getQueue().present(nextImageInfo.image, &renderFinishedSemaphore))
		{
			swapchainOutOfData = true;
		}
	}
}

void Engine::shutdown()
{
	_vkContext->getDevice().waitIdle();
	
	FileHelper::shutdown();
	UIHelper::shutdown();
	_scene.reset();
	_assetManager.reset();
	_window.reset();
	_vkContext.reset();
	glfwTerminate();
}

VKContext& Engine::getVKContext()
{
	return *_vkContext;
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