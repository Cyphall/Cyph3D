#include "Engine.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/UIHelper.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKSwapchainImage.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/Window.h"
#include "VKObject/Fence/VKFence.h"
#include "VKObject/Semaphore/VKSemaphore.h"

#include <GLFW/glfw3.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace
{
void initLogger(spdlog::level::level_enum logLevel)
{
	std::vector<spdlog::sink_ptr> sinks;

	{
		auto& fileSink = sinks.emplace_back(
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("Cyph3D.log")
		);
		fileSink->set_level(spdlog::level::trace);
	}

	{
		auto& stdoutColorSink = sinks.emplace_back(
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		);
		stdoutColorSink->set_level(spdlog::level::trace);
	}

	{
		auto& breakpointSink = sinks.emplace_back(
			std::make_shared<spdlog::sinks::callback_sink_mt>(
				[](const spdlog::details::log_msg&)
				{
					volatile int dummy = 0;
					(void)dummy;
				}
			)
		);
		breakpointSink->set_level(spdlog::level::err);
	}

	std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end());
	logger->set_level(logLevel);
	logger->flush_on(spdlog::level::err);

	spdlog::set_default_logger(std::move(logger));
}
}

std::unique_ptr<c3d::VKContext> c3d::Engine::_vkContext;
std::unique_ptr<c3d::Window> c3d::Engine::_window;
std::unique_ptr<c3d::AssetManager> c3d::Engine::_assetManager;
std::unique_ptr<c3d::Scene> c3d::Engine::_scene;

c3d::Timer c3d::Engine::_timer;

void c3d::Engine::init()
{
#if defined(_DEBUG)
	initLogger(spdlog::level::debug);
#else
	initLogger(spdlog::level::info);
#endif

	glfwInit();

	glfwSetErrorCallback(
		[](int code, const char* message)
		{
			spdlog::error(message);
		}
	);

	_vkContext = VKContext::create(2);

	vk::PhysicalDeviceDriverProperties driverProperties;
	vk::PhysicalDeviceProperties2 properties;
	properties.pNext = &driverProperties;
	_vkContext->getPhysicalDevice().getProperties2(&properties);
	spdlog::info("GPU: {}", static_cast<std::string_view>(properties.properties.deviceName));
	spdlog::info("Driver: {} {}", static_cast<std::string_view>(driverProperties.driverName), static_cast<std::string_view>(driverProperties.driverInfo));

	_window = std::make_unique<Window>();

	_assetManager = std::make_unique<AssetManager>();

	MaterialAsset::initDefaultAndMissing();
	MeshAsset::initDefaultAndMissing();
	Entity::initComponentFactories();

	_scene = std::make_unique<Scene>();

	UIHelper::init();
	FileHelper::init();
}

void c3d::Engine::run()
{
	vk::FenceCreateInfo fenceCreateInfo;
	std::shared_ptr<VKFence> acquireFence = VKFence::create(*_vkContext, fenceCreateInfo);

	std::vector<std::shared_ptr<VKSemaphore>> presentSemaphores;
	presentSemaphores.reserve(3);
	for (int i = 0; i < 3; i++)
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo;
		presentSemaphores.emplace_back(VKSemaphore::create(Engine::getVKContext(), semaphoreCreateInfo));
	}

	while (!_window->shouldClose())
	{
		_vkContext->onNewFrame();
		_assetManager->onNewFrame();

		_vkContext->getDefaultCommandBuffer()->waitExecution();
		_vkContext->getDefaultCommandBuffer()->reset();

		const std::shared_ptr<VKSwapchainImage>& image = _window->getSwapchain().retrieveNextImage(*acquireFence);

		acquireFence->wait();
		acquireFence->reset();

		_timer.onNewFrame();

		glfwPollEvents();
		_window->onPollEvents();

		UIHelper::onNewFrame();

		_scene->onUpdate();

		const std::shared_ptr<VKSemaphore>& presentSemaphore = presentSemaphores[image->getIndex()];
		UIHelper::render(image->getImage(), presentSemaphore);

		if (!_vkContext->getMainQueue().present(image, presentSemaphore))
		{
			glm::uvec2 surfaceSize = _window->getSurfaceSize();

			while (surfaceSize.x * surfaceSize.y == 0)
			{
				glfwWaitEvents();
				surfaceSize = _window->getSurfaceSize();
			}

			_window->recreateSwapchain();
			_vkContext->getDevice().waitIdle();
			presentSemaphores.clear();
			for (int i = 0; i < 3; i++)
			{
				vk::SemaphoreCreateInfo semaphoreCreateInfo;
				presentSemaphores.emplace_back(VKSemaphore::create(Engine::getVKContext(), semaphoreCreateInfo));
			}
		}
	}

	_vkContext->getDevice().waitIdle();
}

void c3d::Engine::shutdown()
{
	_vkContext->getDevice().waitIdle();

	FileHelper::shutdown();
	UIHelper::shutdown();
	_scene.reset();
	_assetManager.reset();
	_window.reset();
	_vkContext.reset();
	glfwTerminate();
	spdlog::shutdown();
}

c3d::VKContext& c3d::Engine::getVKContext()
{
	return *_vkContext;
}

c3d::Window& c3d::Engine::getWindow()
{
	return *_window;
}

c3d::AssetManager& c3d::Engine::getAssetManager()
{
	return *_assetManager;
}

c3d::Scene& c3d::Engine::getScene()
{
	return *_scene;
}

void c3d::Engine::setScene(std::unique_ptr<Scene>&& scene)
{
	UIInspector::setSelected(nullptr);
	_scene = std::move(scene);
}

c3d::Timer& c3d::Engine::getTimer()
{
	return _timer;
}