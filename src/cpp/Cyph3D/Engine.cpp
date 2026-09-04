#include "Engine.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Entity/Entity.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Scene/Scene.h>
#include <Cyph3D/UI/UIHelper.h>
#include <Cyph3D/UI/Window/UIInspector.h>
#include <Cyph3D/UI/Window/UIMisc.h>
#include <Cyph3D/Window.h>

#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/Context.hpp>
#include <CyphGPU/ContextSession.hpp>
#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/ShaderBundle.hpp>
#include <CyphGPU/Swapchain.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

CGPU_DECLARE_SHADER_BUNDLE(shaders)

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
				[](const spdlog::details::log_msg&) {
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

uint32_t rankDevice(const cgpu::DevicePtr& device)
{
	if (device->getVulkanVersion() < cgpu::Context::VULKAN_API_VERSION)
	{
		return 0;
	}

	cgpu::Device::Capabilities requiredCaps;
	requiredCaps |= cgpu::Device::Capability::eCore;
	requiredCaps |= cgpu::Device::Capability::eSwapchain;

	if ((device->getCapabilities() & requiredCaps) != requiredCaps)
	{
		return 0;
	}

	// from here, device is at least compatible

	uint32_t score = 1;

	switch (device->getType())
	{
	case vk::PhysicalDeviceType::eDiscreteGpu: score += 1000; break;
	case vk::PhysicalDeviceType::eIntegratedGpu: score += 100; break;
	default: break;
	}

	if (device->getCapabilities() & cgpu::Device::Capability::eRayTracing)
	{
		score += 1000;
	}

	return score;
}

std::optional<cgpu::DeviceSessionPtr> tryCreateDeviceSession()
{
	cgpu::ContextPtr context = cgpu::Context::create({
		.shader_bundles = {&shaders},
	});

	cgpu::ContextSessionPtr contextSession = cgpu::ContextSession::create(
		context,
		{
			.application_name = "Cyph3D",
		}
	);

	cgpu::Device::Capabilities requiredCaps;
	requiredCaps |= cgpu::Device::Capability::eCore;
	requiredCaps |= cgpu::Device::Capability::eSwapchain;

	std::optional<cgpu::DevicePtr> selectedDevice;
	uint32_t selectedDeviceScore = 0;
	for (const cgpu::DevicePtr& device : contextSession->getDevices())
	{
		uint32_t score = rankDevice(device);
		if (score > selectedDeviceScore)
		{
			selectedDevice = device;
			selectedDeviceScore = score;
		}
	}

	if (!selectedDevice)
	{
		spdlog::error("Could not find a compatible device.");
		return std::nullopt;
	}

	// Create device session
	return cgpu::DeviceSession::create(
		*selectedDevice,
		{}
	);
}
}

cgpu::DeviceSessionPtr c3d::Engine::_deviceSession;
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

	glfwSetErrorCallback([](int code, const char* message) { spdlog::error(message); });

	auto deviceSession = tryCreateDeviceSession();
	if (!deviceSession)
	{
		throw std::runtime_error("Could not find a compatible device.");
	}

	_deviceSession = *deviceSession;

	const auto& device = _deviceSession->getDevice();
	spdlog::info("GPU: {}", device->getDeviceName());
	spdlog::info("Driver: {} {}", device->getDriverName(), device->getDriverInfo());

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
	cgpu::CommandContext commandContext{_deviceSession};
	while (!_window->shouldClose())
	{
		_timer.onNewFrame();

		glfwPollEvents();
		_window->onPollEvents();

		UIHelper::onNewFrame();

		if (!UIMisc::isRenderToFileInProgress())
		{
			_scene->onUpdate();
		}

		_window->ensureValidSwapchain();
		UIHelper::render(commandContext, *_window->getSwapchain()->tryGetImage());

		commandContext.finish();

		_window->getSwapchain()->presentImage();

		FrameMark;
	}
}

void c3d::Engine::shutdown()
{
	_deviceSession->waitIdle();

	FileHelper::shutdown();
	UIHelper::shutdown();
	_scene.reset();
	_assetManager.reset();
	_window.reset();
	_deviceSession.reset();
	glfwTerminate();
	spdlog::shutdown();
}

const cgpu::DeviceSessionPtr& c3d::Engine::getDeviceSession()
{
	return _deviceSession;
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
