#include "VKContext.h"

#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_hash.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

static const uint32_t VULKAN_VERSION = VK_API_VERSION_1_3;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

struct VKContext::HelperData
{
	VKPtr<VKCommandBuffer> immediateCommandBuffer;
	VKDynamic<VKCommandBuffer> defaultCommandBuffer;
};

struct PhysicalDeviceInfo
{
	vk::PhysicalDevice handle;
	vk::PhysicalDeviceProperties properties;
	bool coreLayersSupported;
	bool coreExtensionsSupported;
	bool rayTracingExtensionsSupported;
};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL messageCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageTypes, const vk::DebugUtilsMessengerCallbackDataEXT* messageData, void* userData)
{
	if (messageData->messageIdNumber == 0x48a09f6c) // UNASSIGNED-BestPractices-pipeline-stage-flags
	{
		return false;
	}
	
	switch (messageSeverity)
	{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			Logger::error(messageData->pMessage, "Vulkan");
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			Logger::warning(messageData->pMessage, "Vulkan");
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			Logger::info(messageData->pMessage, "Vulkan");
			break;
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			Logger::debug(messageData->pMessage, "Vulkan");
			break;
	}
	
	return false;
}

static std::vector<const char*> getRequiredInstanceLayers()
{
	std::vector<const char*> layers;

#if defined(_DEBUG)
	layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
	
	return layers;
}

static std::vector<const char*> getRequiredInstanceExtensions()
{
	std::vector<const char*> extensions;
	
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	for (int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}
	
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#if defined(_DEBUG)
	extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
#endif
	
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

static std::unordered_set<std::string> getSupportedInstanceExtensions(const std::unordered_set<std::string>& supportedInstanceLayers)
{
	std::unordered_set<std::string> extensions;
	
	for (const vk::ExtensionProperties& extension : vk::enumerateInstanceExtensionProperties())
	{
		extensions.insert(extension.extensionName);
	}
	
	for (const std::string& layer : supportedInstanceLayers)
	{
		for (const vk::ExtensionProperties& extension : vk::enumerateInstanceExtensionProperties(layer))
		{
			extensions.insert(extension.extensionName);
		}
	}
	
	return extensions;
}

static void checkInstanceLayersSupport(const std::vector<const char*>& requiredInstanceLayers, const std::unordered_set<std::string>& supportedInstanceLayers)
{
	for (const char* requiredInstanceLayer : requiredInstanceLayers)
	{
		if (supportedInstanceLayers.find(std::string(requiredInstanceLayer)) == supportedInstanceLayers.end())
		{
			throw std::runtime_error(std::format("Vulkan instance layer \"{}\" is not supported by this driver.", requiredInstanceLayer));
		}
	}
}

static void checkInstanceExtensionSupport(const std::vector<const char*>& requiredInstanceExtensions, const std::unordered_set<std::string>& supportedInstanceExtensions)
{
	for (const char* requiredInstanceExtension : requiredInstanceExtensions)
	{
		if (supportedInstanceExtensions.find(std::string(requiredInstanceExtension)) == supportedInstanceExtensions.end())
		{
			throw std::runtime_error(std::format("Vulkan instance extension \"{}\" is not supported by this driver.", requiredInstanceExtension));
		}
	}
}

static std::vector<const char*> getRequiredDeviceLayers()
{
	std::vector<const char*> layers;
	
	// none
	
	return layers;
}

static std::vector<const char*> getRequiredDeviceCoreExtensions()
{
	std::vector<const char*> extensions;
	
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
	extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	extensions.push_back(VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME);
	
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

static std::unordered_set<std::string> getSupportedDeviceLayers(vk::PhysicalDevice physicalDevice)
{
	std::unordered_set<std::string> layers;
	
	for (const vk::LayerProperties& layer : physicalDevice.enumerateDeviceLayerProperties())
	{
		layers.insert(layer.layerName);
	}
	
	return layers;
}

static std::unordered_set<std::string> getSupportedDeviceExtensions(vk::PhysicalDevice physicalDevice, const std::unordered_set<std::string>& supportedDeviceLayers)
{
	std::unordered_set<std::string> extensions;
	
	for (const vk::ExtensionProperties& extension : physicalDevice.enumerateDeviceExtensionProperties())
	{
		extensions.insert(extension.extensionName);
	}
	
	for (const std::string& layer : supportedDeviceLayers)
	{
		for (const vk::ExtensionProperties& extension : vk::enumerateInstanceExtensionProperties(layer))
		{
			extensions.insert(extension.extensionName);
		}
	}
	
	return extensions;
}

PhysicalDeviceInfo getPhysicalDeviceInfo(
	vk::PhysicalDevice physicalDevice,
	const std::vector<const char*>& requiredDeviceLayers,
	const std::vector<const char*>& requiredDeviceCoreExtensions,
	const std::vector<const char*>& requiredDeviceRayTracingExtensions)
{
	PhysicalDeviceInfo physicalDeviceInfo;
	physicalDeviceInfo.handle = physicalDevice;
	physicalDeviceInfo.properties = physicalDevice.getProperties();
	
	std::unordered_set<std::string> supportedDeviceLayers = getSupportedDeviceLayers(physicalDevice);
	
	physicalDeviceInfo.coreLayersSupported = true;
	for (const char* requiredDeviceLayer : requiredDeviceLayers)
	{
		if (supportedDeviceLayers.find(std::string(requiredDeviceLayer)) == supportedDeviceLayers.end())
		{
			physicalDeviceInfo.coreLayersSupported = false;
			break;
		}
	}
	
	std::unordered_set<std::string> supportedDeviceExtensions = getSupportedDeviceExtensions(physicalDevice, supportedDeviceLayers);
	
	physicalDeviceInfo.coreExtensionsSupported = true;
	for (const char* requiredDeviceCoreExtension : requiredDeviceCoreExtensions)
	{
		if (supportedDeviceExtensions.find(std::string(requiredDeviceCoreExtension)) == supportedDeviceExtensions.end())
		{
			physicalDeviceInfo.coreExtensionsSupported = false;
			break;
		}
	}
	
	physicalDeviceInfo.rayTracingExtensionsSupported = true;
	for (const char* requiredDeviceRayTracingExtension : requiredDeviceRayTracingExtensions)
	{
		if (supportedDeviceExtensions.find(std::string(requiredDeviceRayTracingExtension)) == supportedDeviceExtensions.end())
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
	
	if (!physicalDevice.coreLayersSupported)
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
	vk::DynamicLoader dynamicLoader;
	VULKAN_HPP_DEFAULT_DISPATCHER.init(dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
	
	std::vector<const char*> requiredInstanceLayers = getRequiredInstanceLayers();
	std::vector<const char*> requiredInstanceExtensions = getRequiredInstanceExtensions();
	std::unordered_set<std::string> supportedInstanceLayers = getSupportedInstanceLayers();
	std::unordered_set<std::string> supportedInstanceExtensions = getSupportedInstanceExtensions(supportedInstanceLayers);
	checkInstanceLayersSupport(requiredInstanceLayers, supportedInstanceLayers);
	checkInstanceExtensionSupport(requiredInstanceExtensions, supportedInstanceExtensions);
	
	createInstance(requiredInstanceLayers, requiredInstanceExtensions);
	
	VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);
	
	createMessenger();
	
	std::vector<const char*> requiredDeviceLayers = getRequiredDeviceLayers();
	std::vector<const char*> requiredDeviceCoreExtensions = getRequiredDeviceCoreExtensions();
	std::vector<const char*> requiredDeviceRayTracingExtensions = getRequiredDeviceRayTracingExtensions();
	std::vector<PhysicalDeviceInfo> physicalDevicesInfos;
	for (vk::PhysicalDevice physicalDevice : _instance.enumeratePhysicalDevices())
	{
		physicalDevicesInfos.push_back(getPhysicalDeviceInfo(physicalDevice, requiredDeviceLayers, requiredDeviceCoreExtensions, requiredDeviceRayTracingExtensions));
	}
	
	if (physicalDevicesInfos.empty())
	{
		throw std::runtime_error("Could not find a device supporting Vulkan.");
	}
	
	const PhysicalDeviceInfo* bestPhysicalDevice = selectPhysicalDevice(physicalDevicesInfos);
	
	if (bestPhysicalDevice == nullptr)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not find a device supporting required Vulkan version, layers and extensions.\n\n";
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
	createLogicalDevice(requiredDeviceLayers, deviceExtensions);
	
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
	_computeQueue->handleCompletedSubmits();
	_transferQueue->handleCompletedSubmits();
	_helperData.reset();
	_vmaAllocator.destroy();
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
	_computeQueue->handleCompletedSubmits();
	_transferQueue->handleCompletedSubmits();
	_vmaAllocator.setCurrentFrameIndex(_currentConcurrentFrame);
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
	return *_computeQueue;
}

VKQueue& VKContext::getTransferQueue()
{
	return *_transferQueue;
}

vma::Allocator VKContext::getVmaAllocator()
{
	return _vmaAllocator;
}

const VKPtr<VKCommandBuffer>& VKContext::getDefaultCommandBuffer()
{
	return _helperData->defaultCommandBuffer.getCurrent();
}

void VKContext::executeImmediate(std::function<void(const VKPtr<VKCommandBuffer>& commandBuffer)>&& function)
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

#if defined(_DEBUG)
	std::vector<vk::ValidationFeatureEnableEXT> enabledFeatures{
		vk::ValidationFeatureEnableEXT::eGpuAssisted,
		vk::ValidationFeatureEnableEXT::eBestPractices,
		vk::ValidationFeatureEnableEXT::eSynchronizationValidation
	};
	
	vk::ValidationFeaturesEXT validationFeatures;
	validationFeatures.enabledValidationFeatureCount = enabledFeatures.size();
	validationFeatures.pEnabledValidationFeatures = enabledFeatures.data();
	
	createInfo.pNext = &validationFeatures;
#endif
	
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

bool VKContext::findBestQueueFamilies(uint32_t& mainQueueFamily, uint32_t& computeQueueFamily, uint32_t& transferQueueFamily)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
	
	struct QueueFamilyInfo
	{
		uint32_t index{};
		uint32_t usageCount{};
		vk::QueueFlags usages{};
	};
	
	std::vector<QueueFamilyInfo> queueFamilyInfos(queueFamilyProperties.size());
	
	for (int i = 0; i < queueFamilyProperties.size(); i++)
	{
		queueFamilyInfos[i].index = i;
		
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queueFamilyInfos[i].usages |= vk::QueueFlagBits::eGraphics;
			queueFamilyInfos[i].usageCount++;
		}
		
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
		{
			queueFamilyInfos[i].usages |= vk::QueueFlagBits::eCompute;
			queueFamilyInfos[i].usageCount++;
		}
		
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
		{
			queueFamilyInfos[i].usages |= vk::QueueFlagBits::eTransfer;
			queueFamilyInfos[i].usageCount++;
		}
	}
	
	std::sort(queueFamilyInfos.begin(), queueFamilyInfos.end(), [](const QueueFamilyInfo& a, const QueueFamilyInfo& b)
	{
		return a.usageCount < b.usageCount;
	});
	
	bool mainQueueFamilyFound = false;
	bool computeQueueFamilyFound = false;
	bool transferQueueFamilyFound = false;
	for (const QueueFamilyInfo& queueFamilyInfo : queueFamilyInfos)
	{
		if (!mainQueueFamilyFound &&
			queueFamilyInfo.usages & vk::QueueFlagBits::eGraphics &&
			queueFamilyInfo.usages & vk::QueueFlagBits::eCompute &&
			queueFamilyInfo.usages & vk::QueueFlagBits::eTransfer)
		{
			mainQueueFamily = queueFamilyInfo.index;
			mainQueueFamilyFound = true;
		}
		
		if (!computeQueueFamilyFound &&
		    queueFamilyInfo.usages & vk::QueueFlagBits::eCompute)
		{
			computeQueueFamily = queueFamilyInfo.index;
			computeQueueFamilyFound = true;
		}
		
		if (!transferQueueFamilyFound &&
		    queueFamilyInfo.usages & vk::QueueFlagBits::eTransfer)
		{
			transferQueueFamily = queueFamilyInfo.index;
			transferQueueFamilyFound = true;
		}
	}
	
	return mainQueueFamilyFound && computeQueueFamilyFound && transferQueueFamilyFound;
}

void VKContext::createLogicalDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	uint32_t mainQueueFamily;
	uint32_t computeQueueFamily;
	uint32_t transferQueueFamily;
	if (!findBestQueueFamilies(mainQueueFamily, computeQueueFamily, transferQueueFamily))
	{
		throw;
	}
	
	float mainQueuePriority = 1.0f;
	float computeQueuePriority = 0.5f;
	float transferQueuePriority = 0.5f;
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
	
	{
		vk::DeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos.emplace_back();
		createInfo.queueFamilyIndex = mainQueueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &mainQueuePriority;
	}
	
	{
		vk::DeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos.emplace_back();
		createInfo.queueFamilyIndex = computeQueueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &computeQueuePriority;
	}
	
	{
		vk::DeviceQueueCreateInfo& createInfo = deviceQueueCreateInfos.emplace_back();
		createInfo.queueFamilyIndex = transferQueueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &transferQueuePriority;
	}
	
	vk::PhysicalDeviceShaderImageAtomicInt64FeaturesEXT shaderImageAtomicInt64Features;
	shaderImageAtomicInt64Features.shaderImageInt64Atomics = true;
	
	vk::PhysicalDeviceScalarBlockLayoutFeatures scalarBlockLayoutFeatures;
	scalarBlockLayoutFeatures.scalarBlockLayout = true;
	scalarBlockLayoutFeatures.pNext = &shaderImageAtomicInt64Features;
	
	vk::PhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
	uniformBufferStandardLayoutFeatures.uniformBufferStandardLayout = true;
	uniformBufferStandardLayoutFeatures.pNext = &scalarBlockLayoutFeatures;
	
	vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenance1Features;
	rayTracingMaintenance1Features.rayTracingMaintenance1 = true;
	rayTracingMaintenance1Features.pNext = &uniformBufferStandardLayoutFeatures;
	
	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures;
	rayTracingPipelineFeatures.rayTracingPipeline = true;
	rayTracingPipelineFeatures.pNext = &rayTracingMaintenance1Features;
	
	vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
	accelerationStructureFeatures.accelerationStructure = true;
	accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
	
	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	bufferDeviceAddressFeatures.bufferDeviceAddress = true;
	
	if (_rayTracingSupported)
	{
		bufferDeviceAddressFeatures.pNext = &accelerationStructureFeatures;
	}
	
	vk::PhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures;
	hostQueryResetFeatures.hostQueryReset = true;
	hostQueryResetFeatures.pNext = &bufferDeviceAddressFeatures;
	
	vk::PhysicalDeviceRobustness2FeaturesEXT robustness2Features;
	robustness2Features.nullDescriptor = true;
	robustness2Features.pNext = &hostQueryResetFeatures;
	
	vk::PhysicalDeviceMaintenance4Features maintenance4Features;
	maintenance4Features.maintenance4 = true;
	maintenance4Features.pNext = &robustness2Features;
	
	vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures;
	descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
	descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
	descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
	descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = true;
	descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
	descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
	descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = true;
	descriptorIndexingFeatures.runtimeDescriptorArray = true;
	descriptorIndexingFeatures.pNext = &maintenance4Features;
	
	vk::PhysicalDeviceSynchronization2Features synchronization2Features;
	synchronization2Features.synchronization2 = true;
	synchronization2Features.pNext = &descriptorIndexingFeatures;
	
	vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
	dynamicRenderingFeatures.pNext = &synchronization2Features;
	dynamicRenderingFeatures.dynamicRendering = true;
	
	vk::PhysicalDeviceFeatures2 physicalDeviceFeatures;
	physicalDeviceFeatures.pNext = &dynamicRenderingFeatures;
	physicalDeviceFeatures.features.samplerAnisotropy = true;
	physicalDeviceFeatures.features.geometryShader = true;
	physicalDeviceFeatures.features.shaderStorageImageReadWithoutFormat = true;
	physicalDeviceFeatures.features.shaderStorageImageWriteWithoutFormat = true;
	physicalDeviceFeatures.features.shaderInt64 = true;
	
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.pNext = &physicalDeviceFeatures;
	deviceCreateInfo.queueCreateInfoCount = deviceQueueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
	deviceCreateInfo.enabledLayerCount = layers.size();
	deviceCreateInfo.ppEnabledLayerNames = layers.data();
	
	_device = _physicalDevice.createDevice(deviceCreateInfo);
	
	_mainQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, mainQueueFamily, 0));
	_computeQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, computeQueueFamily, 0));
	_transferQueue = std::unique_ptr<VKQueue>(new VKQueue(*this, transferQueueFamily, 0));
}

void VKContext::createVmaAllocator()
{
	vma::VulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;
	
	vma::AllocatorCreateInfo allocatorInfo{};
	allocatorInfo.vulkanApiVersion = VULKAN_VERSION;
	allocatorInfo.instance = _instance;
	allocatorInfo.physicalDevice = _physicalDevice;
	allocatorInfo.device = _device;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;
	allocatorInfo.flags = vma::AllocatorCreateFlagBits::eExtMemoryBudget | vma::AllocatorCreateFlagBits::eBufferDeviceAddress;
	
	vma::createAllocator(&allocatorInfo, &_vmaAllocator);
}

void VKContext::createImmediateCommandBuffer()
{
	_helperData->immediateCommandBuffer = VKCommandBuffer::create(*this, *_mainQueue);
}

void VKContext::createDefaultCommandBuffer()
{
	_helperData->defaultCommandBuffer = VKDynamic<VKCommandBuffer>(*this, [](VKContext& context, int index)
	{
		return VKCommandBuffer::create(context, context.getMainQueue());
	});
}