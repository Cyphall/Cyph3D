#include "Engine.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/ThreadHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/UIHelper.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKSwapchainImage.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/Window.h"

#include <GLFW/glfw3.h>

std::unique_ptr<VKContext> Engine::_vkContext;
std::unique_ptr<Window> Engine::_window;
std::unique_ptr<AssetManager> Engine::_assetManager;
std::unique_ptr<Scene> Engine::_scene;

Timer Engine::_timer;

void Engine::init()
{
#if defined(_DEBUG)
	Logger::init(Logger::LogLevel::eDebug);
#else
	Logger::init(Logger::LogLevel::eInfo);
#endif

	glfwInit();

	glfwSetErrorCallback(
		[](int code, const char* message)
		{
			Logger::error(message);
		}
	);

	_vkContext = VKContext::create(2);

	vk::PhysicalDeviceDriverProperties driverProperties;
	vk::PhysicalDeviceProperties2 properties;
	properties.pNext = &driverProperties;
	_vkContext->getPhysicalDevice().getProperties2(&properties);
	Logger::info("GPU: {}", static_cast<std::string_view>(properties.properties.deviceName));
	Logger::info("Driver: {} {}", static_cast<std::string_view>(driverProperties.driverName), static_cast<std::string_view>(driverProperties.driverInfo));

	_window = std::make_unique<Window>();

	_assetManager = std::make_unique<AssetManager>(std::max(ThreadHelper::getPhysicalCoreCount() - 1, 1));

	MaterialAsset::initDefaultAndMissing();
	MeshAsset::initDefaultAndMissing();
	Entity::initComponentFactories();

	_scene = std::make_unique<Scene>();

	UIHelper::init();
	FileHelper::init();
}

void Engine::run()
{
	while (!_window->shouldClose())
	{
		_vkContext->onNewFrame();
		_assetManager->onNewFrame();

		_vkContext->getDefaultCommandBuffer()->waitExecution();
		_vkContext->getDefaultCommandBuffer()->reset();

		VKSwapchain::NextImageInfo nextImageInfo = _window->getSwapchain().retrieveNextImage();

		_timer.onNewFrame();

		glfwPollEvents();
		_window->onPollEvents();

		UIHelper::onNewFrame();

		_scene->onUpdate();

		const std::shared_ptr<VKSemaphore>& renderFinishedSemaphore = UIHelper::render(nextImageInfo.image->getImage(), nextImageInfo.imageAvailableSemaphore);
		if (!_vkContext->getMainQueue().present(nextImageInfo.image, renderFinishedSemaphore))
		{
			glm::uvec2 surfaceSize = _window->getSurfaceSize();

			while (surfaceSize.x * surfaceSize.y == 0)
			{
				glfwWaitEvents();
				surfaceSize = _window->getSurfaceSize();
			}

			_window->recreateSwapchain();
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