#include "VKContext.h"

#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <fstream>
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

static constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_3;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

struct VKContext::HelperData
{
	std::shared_ptr<VKCommandBuffer> immediateCommandBuffer;
	VKDynamic<VKCommandBuffer> defaultCommandBuffer;
};

struct PhysicalDeviceInfo
{
	vk::PhysicalDevice handle;
	vk::PhysicalDeviceProperties properties;
	bool coreExtensionsSupported;
	bool rayTracingExtensionsSupported;
};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL messageCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageTypes, const vk::DebugUtilsMessengerCallbackDataEXT* messageData, void* userData)
{
	switch (messageSeverity)
	{
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		Logger::error(messageData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		Logger::warning(messageData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		Logger::info(messageData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		Logger::debug(messageData->pMessage);
		break;
	}

	return false;
}

static std::vector<const char*> getRequiredInstanceLayers()
{
	std::vector<const char*> layers;

	return layers;
}

static std::vector<const char*> getPreferredInstanceLayers()
{
	std::vector<const char*> layers;

	layers.push_back("VK_LAYER_LUNARG_monitor");

	return layers;
}

static std::vector<const char*> getRequiredInstanceExtensions()
{
	std::vector<const char*> extensions;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);

	return extensions;
}

static std::vector<const char*> getPreferredInstanceExtensions()
{
	std::vector<const char*> extensions;

	// none

	return extensions;
}

static std::unordered_set<std::string> getSupportedInstanceLayers()
{
	std::unordered_set<std::string> layers;

	for (const vk::LayerProperties& layer : vk::enumerateInstanceLayerProperties())
	{
		layers.insert(layer.layerName);
	}

	return layers;
}

static std::unordered_set<std::string> getSupportedInstanceExtensions(const std::vector<const char*>& instanceLayers)
{
	std::unordered_set<std::string> extensions;

	for (const vk::ExtensionProperties& extension : vk::enumerateInstanceExtensionProperties())
	{
		extensions.insert(extension.extensionName);
	}

	for (const char* layer : instanceLayers)
	{
		for (const vk::ExtensionProperties& extension : vk::enumerateInstanceExtensionProperties(std::string(layer)))
		{
			extensions.insert(extension.extensionName);
		}
	}

	return extensions;
}

static std::vector<const char*> buildInstanceLayerList(const std::vector<const char*>& requiredInstanceLayers, const std::vector<const char*>& preferredInstanceLayers, const std::unordered_set<std::string>& supportedInstanceLayers)
{
	std::vector<const char*> layers;

	for (const char* requiredInstanceLayer : requiredInstanceLayers)
	{
		if (!supportedInstanceLayers.contains(std::string(requiredInstanceLayer)))
		{
			throw std::runtime_error(std::format("Vulkan instance layer \"{}\" is not available.", requiredInstanceLayer));
		}

		layers.emplace_back(requiredInstanceLayer);
	}

	for (const char* preferredInstanceLayer : preferredInstanceLayers)
	{
		if (!supportedInstanceLayers.contains(std::string(preferredInstanceLayer)))
		{
			continue;
		}

		layers.emplace_back(preferredInstanceLayer);
	}

	return layers;
}

static std::vector<const char*> buildInstanceExtensionList(const std::vector<const char*>& requiredInstanceExtensions, const std::vector<const char*>& preferredInstanceExtensions, const std::unordered_set<std::string>& supportedInstanceExtensions)
{
	std::vector<const char*> extensions;

	for (const char* requiredInstanceExtension : requiredInstanceExtensions)
	{
		if (!supportedInstanceExtensions.contains(std::string(requiredInstanceExtension)))
		{
			throw std::runtime_error(std::format("Vulkan instance extension \"{}\" is not available.", requiredInstanceExtension));
		}

		extensions.emplace_back(requiredInstanceExtension);
	}

	for (const char* preferredInstanceExtension : preferredInstanceExtensions)
	{
		if (!supportedInstanceExtensions.contains(std::string(preferredInstanceExtension)))
		{
			continue;
		}

		extensions.emplace_back(preferredInstanceExtension);
	}

	return extensions;
}

static std::vector<const char*> getRequiredDeviceCoreExtensions()
{
	std::vector<const char*> extensions;

	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
	extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	extensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
	extensions.push_back(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);

	return extensions;
}

static std::vector<const char*> getRequiredDeviceRayTracingExtensions()
{
	std::vector<const char*> extensions;

	extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
	extensions.push_back(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);

	return extensions;
}

static std::unordered_set<std::string> getSupportedDeviceExtensions(vk::PhysicalDevice physicalDevice)
{
	std::unordered_set<std::string> extensions;

	for (const vk::ExtensionProperties& extension : physicalDevice.enumerateDeviceExtensionProperties())
	{
		extensions.insert(extension.extensionName);
	}

	return extensions;
}

static PhysicalDeviceInfo getPhysicalDeviceInfo(
	vk::PhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceCoreExtensions,
	const std::vector<const char*>& requiredDeviceRayTracingExtensions
)
{
	PhysicalDeviceInfo physicalDeviceInfo;
	physicalDeviceInfo.handle = physicalDevice;
	physicalDeviceInfo.properties = physicalDevice.getProperties();

	std::unordered_set<std::string> supportedDeviceExtensions = getSupportedDeviceExtensions(physicalDevice);

	physicalDeviceInfo.coreExtensionsSupported = true;
	for (const char* requiredDeviceCoreExtension : requiredDeviceCoreExtensions)
	{
		if (!supportedDeviceExtensions.contains(std::string(requiredDeviceCoreExtension)))
		{
			physicalDeviceInfo.coreExtensionsSupported = false;
			break;
		}
	}

	physicalDeviceInfo.rayTracingExtensionsSupported = true;
	for (const char* requiredDeviceRayTracingExtension : requiredDeviceRayTracingExtensions)
	{
		if (!supportedDeviceExtensions.contains(std::string(requiredDeviceRayTracingExtension)))
		{
			physicalDeviceInfo.rayTracingExtensionsSupported = false;
			break;
		}
	}

	return physicalDeviceInfo;
}

static int calculateDeviceScore(const PhysicalDeviceInfo& physicalDevice)
{
	if (physicalDevice.properties.apiVersion < VULKAN_VERSION)
	{
		return 0;
	}

	if (!physicalDevice.coreExtensionsSupported)
	{
		return 0;
	}

	// from here, device is at least compatible

	int score = 1;

	if (physicalDevice.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
	{
		score += 1000;
	}
	else if (physicalDevice.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
	{
		score += 100;
	}

	if (physicalDevice.rayTracingExtensionsSupported)
	{
		score += 1000;
	}

	return score;
}

static const PhysicalDeviceInfo* selectPhysicalDevice(const std::vector<PhysicalDeviceInfo>& physicalDevices)
{
	const PhysicalDeviceInfo* bestPhysicalDevice = nullptr;
	int bestPhysicalDeviceScore = 0;
	for (const PhysicalDeviceInfo& physicalDevice : physicalDevices)
	{
		int physicalDeviceScore = calculateDeviceScore(physicalDevice);
		if (physicalDeviceScore > bestPhysicalDeviceScore)
		{
			bestPhysicalDevice = &physicalDevice;
			bestPhysicalDeviceScore = physicalDeviceScore;
		}
	}

	return bestPhysicalDevice;
}

std::unique_ptr<VKContext> VKContext::create(int concurrentFrameCount)
{
	return std::unique_ptr<VKContext>(new VKContext(concurrentFrameCount));
}

VKContext::VKContext(int concurrentFrameCount):
	_concurrentFrameCount(concurrentFrameCount)
{
	VULKAN_HPP_DEFAULT_DISPATCHER.init();

	std::vector<const char*> requiredInstanceLayers = getRequiredInstanceLayers();
	std::vector<const char*> preferredInstanceLayers = getPreferredInstanceLayers();
	std::unordered_set<std::string> supportedInstanceLayers = getSupportedInstanceLayers();
	std::vector<const char*> instanceLayers = buildInstanceLayerList(requiredInstanceLayers, preferredInstanceLayers, supportedInstanceLayers);

	std::vector<const char*> requiredInstanceExtensions = getRequiredInstanceExtensions();
	std::vector<const char*> preferredInstanceExtensions = getPreferredInstanceExtensions();
	std::unordered_set<std::string> supportedInstanceExtensions = getSupportedInstanceExtensions(instanceLayers);
	std::vector<const char*> instanceExtensions = buildInstanceExtensionList(requiredInstanceExtensions, preferredInstanceExtensions, supportedInstanceExtensions);

	createInstance(instanceLayers, instanceExtensions);

	VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);

	createMessenger();

	std::vector<const char*> requiredDeviceCoreExtensions = getRequiredDeviceCoreExtensions();
	std::vector<const char*> requiredDeviceRayTracingExtensions = getRequiredDeviceRayTracingExtensions();
	std::vector<PhysicalDeviceInfo> physicalDevicesInfos;
	for (vk::PhysicalDevice physicalDevice : _instance.enumeratePhysicalDevices())
	{
		physicalDevicesInfos.push_back(getPhysicalDeviceInfo(physicalDevice, requiredDeviceCoreExtensions, requiredDeviceRayTracingExtensions));
	}

	if (physicalDevicesInfos.empty())
	{
		throw std::runtime_error("Could not find a device supporting Vulkan.");
	}

	const PhysicalDeviceInfo* bestPhysicalDevice = selectPhysicalDevice(physicalDevicesInfos);

	if (bestPhysicalDevice == nullptr)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not find a device supporting required Vulkan version and extensions.\n\n";
		errorMessage << "Required Vulkan version: " << VK_API_VERSION_MAJOR(VULKAN_VERSION) << '.' << VK_API_VERSION_MINOR(VULKAN_VERSION) << "\n\n";
		errorMessage << "Required Vulkan device extensions:\n";
		for (const char* extension : requiredDeviceCoreExtensions)
		{
			errorMessage << '\t' << extension << '\n';
		}
		errorMessage << '\n';
		throw std::runtime_error(errorMessage.str());
	}

	_physicalDevice = bestPhysicalDevice->handle;
	_rayTracingSupported = bestPhysicalDevice->rayTracingExtensionsSupported;

	std::vector<const char*> deviceExtensions = requiredDeviceCoreExtensions;
	if (_rayTracingSupported)
	{
		deviceExtensions.insert(deviceExtensions.end(), requiredDeviceRayTracingExtensions.begin(), requiredDeviceRayTracingExtensions.end());
	}
	createLogicalDevice(deviceExtensions);

	VULKAN_HPP_DEFAULT_DISPATCHER.init(_device);

	createVmaAllocator();

	_helperData = std::make_unique<VKContext::HelperData>();

	createImmediateCommandBuffer();
	createDefaultCommandBuffer();

	if (_rayTracingSupported)
	{
		_accelerationStructureProperties.pNext = &_rayTracingPipelineProperties;
		_pushDescriptorProperties.pNext = &_accelerationStructureProperties;
	}
	_descriptorIndexingProperties.pNext = &_pushDescriptorProperties;
	_properties.pNext = &_descriptorIndexingProperties;
	_physicalDevice.getProperties2(&_properties);
}

VKContext::~VKContext()
{
	_mainQueue->handleCompletedSubmits();
	if (_computeQueue)
		_computeQueue->handleCompletedSubmits();
	if (_transferQueue)
		_transferQueue->handleCompletedSubmits();
	_helperData.reset();
	vmaDestroyAllocator(_vmaAllocator);
	_device.destroy();
	_instance.destroyDebugUtilsMessengerEXT(_messenger);
	_instance.destroy();
}

int VKContext::getConcurrentFrameCount() const
{
	return _concurrentFrameCount;
}

int VKContext::getCurrentConcurrentFrame() const
{
	return _currentConcurrentFrame;
}

void VKContext::onNewFrame()
{
	_currentConcurrentFrame = (_currentConcurrentFrame + 1) % _concurrentFrameCount;
	_mainQueue->handleCompletedSubmits();
	if (_computeQueue)
		_computeQueue->handleCompletedSubmits();
	if (_transferQueue)
		_transferQueue->handleCompletedSubmits();
	vmaSetCurrentFrameIndex(_vmaAllocator, _currentConcurrentFrame);
}

const vk::Instance& VKContext::getInstance()
{
	return _instance;
}

const vk::PhysicalDevice& VKContext::getPhysicalDevice()
{
	return _physicalDevice;
}

const vk::Device& VKContext::getDevice()
{
	return _device;
}

VKQueue& VKContext::getMainQueue()
{
	return *_mainQueue;
}

VKQueue& VKContext::getComputeQueue()
{
	return _computeQueue ? *_computeQueue : *_mainQueue;
}

VKQueue& VKContext::getTransferQueue()
{
	return _transferQueue ? *_transferQueue : *_mainQueue;
}

VmaAllocator VKContext::getVmaAllocator()
{
	return _vmaAllocator;
}

const std::shared_ptr<VKCommandBuffer>& VKContext::getDefaultCommandBuffer()
{
	return _helperData->defaultCommandBuffer.getCurrent();
}

void VKContext::executeImmediate(std::function<void(const std::shared_ptr<VKCommandBuffer>& commandBuffer)>&& function)
{
	_helperData->immediateCommandBuffer->begin();

	function(_helperData->immediateCommandBuffer);

	_helperData->immediateCommandBuffer->end();

	_mainQueue->submit(_helperData->immediateCommandBuffer, {}, {});

	_helperData->immediateCommandBuffer->waitExecution();
	_helperData->immediateCommandBuffer->reset();
}

const vk::PhysicalDeviceProperties& VKContext::getProperties() const
{
	return _properties.properties;
}

const vk::PhysicalDeviceDescriptorIndexingProperties& VKContext::getDescriptorIndexingProperties() const
{
	return _descriptorIndexingProperties;
}

const vk::PhysicalDevicePushDescriptorPropertiesKHR& VKContext::getPushDescriptorProperties() const
{
	return _pushDescriptorProperties;
}

const vk::PhysicalDeviceAccelerationStructurePropertiesKHR& VKContext::getAccelerationStructureProperties() const
{
	return _accelerationStructureProperties;
}

const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& VKContext::getRayTracingPipelineProperties() const
{
	return _rayTracingPipelineProperties;
}

bool VKContext::isRayTracingSupported() const
{
	return _rayTracingSupported;
}

void VKContext::createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "Cyph3D";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Cyph3D";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VULKAN_VERSION;

	vk::InstanceCreateInfo createInfo;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();

	_instance = vk::createInstance(createInfo);
}

void VKContext::createMessenger()
{
	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	createInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(messageCallback);
	createInfo.pUserData = nullptr; // Optional

	_messenger = _instance.createDebugUtilsMessengerEXT(createInfo);
}

std::vector<VKContext::QueueFamilyInfo> VKContext::parseQueues()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

	std::vector<QueueFamilyInfo> queueFamilyInfos;

	for (int i = 0; i < queueFamilyProperties.size(); i++)
	{
		QueueFamilyInfo& queueFamilyInfo = queueFamilyInfos.emplace_back();
		queueFamilyInfo.index = i;
		queueFamilyInfo.totalQueues = queueFamilyProperties[i].queueCount;
		queueFamilyInfo.usedQueues = 0;

		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queueFamilyInfo.usages |= vk::QueueFlagBits::eGraphics;
			queueFamilyInfo.usageCount++;
		}

		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
		{
			queueFamilyInfo.usages |= vk::QueueFlagBits::eCompute;
			queueFamilyInfo.usageCount++;
		}

		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
		{
			queueFamilyInfo.usages |= vk::QueueFlagBits::eTransfer;
			queueFamilyInfo.usageCount++;
		}
	}

	std::ranges::sort(
		queueFamilyInfos,
		[](const QueueFamilyInfo& a, const QueueFamilyInfo& b)
		{
			return a.usageCount < b.usageCount;
		}
	);

	return queueFamilyInfos;
}

std::optional<VKContext::QueueID> VKContext::findBestQueue(std::vector<QueueFamilyInfo>& queueFamilyInfos, vk::QueueFlags requiredFlags, float priority)
{
	for (QueueFamilyInfo& queueFamilyInfo : queueFamilyInfos)
	{
		if ((queueFamilyInfo.usages & requiredFlags) == requiredFlags && queueFamilyInfo.usedQueues < queueFamilyInfo.totalQueues)
		{
			return QueueID{
				.family = queueFamilyInfo.index,
				.index = queueFamilyInfo.usedQueues++,
				.priority = priority
			};
		}
	}

	return std::nullopt;
}

void VKContext::createLogicalDevice(const std::vector<const char*>& extensions)
{
	std::vector<QueueFamilyInfo> queueFamilyInfos = parseQueues();

	std::unordered_map<uint32_t, std::vector<float>> queueFamilyUsages;

	std::optional<QueueID> mainQueue = findBestQueue(queueFamilyInfos, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer, 1.0f);
	if (mainQueue)
	{
		queueFamilyUsages[mainQueue->family].emplace_back(mainQueue->priority);
	}

	std::optional<QueueID> computeQueue = findBestQueue(queueFamilyInfos, vk::QueueFlagBits::eCompute, 0.0f);
	if (computeQueue)
	{
		queueFamilyUsages[computeQueue->family].emplace_back(computeQueue->priority);
	}

	std::optional<QueueID> transferQueue = findBestQueue(queueFamilyInfos, vk::QueueFlagBits::eTransfer, 0.0f);
	if (transferQueue)
	{
		queueFamilyUsages[transferQueue->family].emplace_back(transferQueue->priority);
	}

	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;

	for (const auto& [queueFamilyIndex, queueFamilyUsage] : queueFamilyUsages)
	{
		vk::DeviceQueueCreateInfo& deviceQueueCreateInfo = deviceQueueCreateInfos.emplace_back();
		deviceQueueCreateInfo.flags = {};
		deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		deviceQueueCreateInfo.queueCount = queueFamilyUsage.size();
		deviceQueueCreateInfo.pQueuePriorities = queueFamilyUsage.data();
	}

	vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceMemoryPriorityFeaturesEXT,
		vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT,
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
		vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR,
		vk::PhysicalDeviceShaderImageAtomicInt64FeaturesEXT>
		features;

	features.unlink<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
	features.unlink<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
	features.unlink<vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR>();
	features.unlink<vk::PhysicalDeviceShaderImageAtomicInt64FeaturesEXT>();

	features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy = true;
	features.get<vk::PhysicalDeviceFeatures2>().features.geometryShader = true;
	features.get<vk::PhysicalDeviceFeatures2>().features.shaderStorageImageReadWithoutFormat = true;
	features.get<vk::PhysicalDeviceFeatures2>().features.shaderStorageImageWriteWithoutFormat = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().shaderUniformBufferArrayNonUniformIndexing = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().shaderSampledImageArrayNonUniformIndexing = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageBufferArrayNonUniformIndexing = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageImageArrayNonUniformIndexing = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().descriptorBindingSampledImageUpdateAfterBind = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().descriptorBindingPartiallyBound = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().descriptorBindingVariableDescriptorCount = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().runtimeDescriptorArray = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().hostQueryReset = true;
	features.get<vk::PhysicalDeviceVulkan12Features>().bufferDeviceAddress = true;
	features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
	features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;
	features.get<vk::PhysicalDeviceVulkan13Features>().maintenance4 = true;
	features.get<vk::PhysicalDeviceMemoryPriorityFeaturesEXT>().memoryPriority = true;
	features.get<vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>().pageableDeviceLocalMemory = true;

	if (_rayTracingSupported)
	{
		features.relink<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
		features.relink<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
		features.relink<vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR>();
		features.relink<vk::PhysicalDeviceShaderImageAtomicInt64FeaturesEXT>();

		features.get<vk::PhysicalDeviceFeatures2>().features.shaderInt64 = true;
		features.get<vk::PhysicalDeviceFeatures2>().features.shaderFloat64 = true;
		features.get<vk::PhysicalDeviceVulkan12Features>().scalarBlockLayout = true;
		features.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>().accelerationStructure = true;
		features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>().rayTracingPipeline = true;
		features.get<vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR>().rayTracingMaintenance1 = true;
		features.get<vk::PhysicalDeviceShaderImageAtomicInt64FeaturesEXT>().shaderImageInt64Atomics = true;
	}

	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.pNext = &features.get<vk::PhysicalDeviceFeatures2>();
	deviceCreateInfo.queueCreateInfoCount = deviceQueueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;

	_device = _physicalDevice.createDevice(deviceCreateInfo);

	if (mainQueue)
	{
		_mainQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, mainQueue->family, mainQueue->index));
	}
	if (computeQueue)
	{
		_computeQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, computeQueue->family, computeQueue->index));
	}
	if (transferQueue)
	{
		_transferQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, transferQueue->family, transferQueue->index));
	}
}

void VKContext::createVmaAllocator()
{
	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.flags =
		VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
		VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT |
		VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT |
		VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
	allocatorInfo.physicalDevice = _physicalDevice;
	allocatorInfo.device = _device;
	allocatorInfo.preferredLargeHeapBlockSize = 0;
	allocatorInfo.pAllocationCallbacks = nullptr;
	allocatorInfo.pDeviceMemoryCallbacks = nullptr;
	allocatorInfo.pHeapSizeLimit = nullptr;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;
	allocatorInfo.instance = _instance;
	allocatorInfo.vulkanApiVersion = VULKAN_VERSION;
	allocatorInfo.pTypeExternalMemoryHandleTypes = nullptr;

	vmaCreateAllocator(&allocatorInfo, &_vmaAllocator);
}

void VKContext::createImmediateCommandBuffer()
{
	_helperData->immediateCommandBuffer = VKCommandBuffer::create(*this, *_mainQueue);
}

void VKContext::createDefaultCommandBuffer()
{
	_helperData->defaultCommandBuffer = VKDynamic<VKCommandBuffer>(
		*this,
		[](VKContext& context, int index)
		{
			return VKCommandBuffer::create(context, context.getMainQueue());
		}
	);
}