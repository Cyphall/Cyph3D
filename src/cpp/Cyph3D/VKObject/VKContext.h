#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace c3d
{
class VKQueue;
class VKCommandBuffer;

class VKContext
{
public:
	static std::unique_ptr<VKContext> create(int concurrentFrameCount);

	~VKContext();

	int getConcurrentFrameCount() const;
	int getCurrentConcurrentFrame() const;

	void onNewFrame();

	const vk::Instance& getInstance();
	const vk::PhysicalDevice& getPhysicalDevice();
	const vk::Device& getDevice();
	VKQueue& getMainQueue();
	VKQueue& getComputeQueue();
	VKQueue& getTransferQueue();

	VmaAllocator getVmaAllocator();

	const std::shared_ptr<VKCommandBuffer>& getDefaultCommandBuffer();

	void executeImmediate(std::function<void(const std::shared_ptr<VKCommandBuffer>& commandBuffer)>&& function);

	const vk::PhysicalDeviceProperties& getVulkan10Properties() const;
	const vk::PhysicalDeviceVulkan11Properties& getVulkan11Properties() const;
	const vk::PhysicalDeviceVulkan12Properties& getVulkan12Properties() const;
	const vk::PhysicalDeviceVulkan13Properties& getVulkan13Properties() const;
	const vk::PhysicalDevicePushDescriptorPropertiesKHR& getPushDescriptorProperties() const;
	const vk::PhysicalDeviceAccelerationStructurePropertiesKHR& getAccelerationStructureProperties() const;
	const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& getRayTracingPipelineProperties() const;

	bool isRayTracingSupported() const;

private:
	struct HelperData;

	struct QueueFamilyInfo
	{
		uint32_t index;
		uint32_t totalQueues;
		uint32_t usedQueues;
		uint32_t usageCount;
		vk::QueueFlags usages;
	};

	struct QueueID
	{
		uint32_t family;
		uint32_t index;
		float priority;
	};

	int _concurrentFrameCount;
	int _currentConcurrentFrame = 0;

	vk::Instance _instance;

	vk::DebugUtilsMessengerEXT _messenger;
	vk::PhysicalDevice _physicalDevice;
	std::unique_ptr<VKQueue> _mainQueue;
	std::unique_ptr<VKQueue> _computeQueue;
	std::unique_ptr<VKQueue> _transferQueue;
	vk::Device _device;

	VmaAllocator _vmaAllocator = VK_NULL_HANDLE;

	std::unique_ptr<HelperData> _helperData;

	vk::PhysicalDeviceProperties2 _vulkan10Properties;
	vk::PhysicalDeviceVulkan11Properties _vulkan11Properties;
	vk::PhysicalDeviceVulkan12Properties _vulkan12Properties;
	vk::PhysicalDeviceVulkan13Properties _vulkan13Properties;
	vk::PhysicalDevicePushDescriptorPropertiesKHR _pushDescriptorProperties;
	vk::PhysicalDeviceAccelerationStructurePropertiesKHR _accelerationStructureProperties;
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR _rayTracingPipelineProperties;

	bool _rayTracingSupported;

	explicit VKContext(int concurrentFrameCount);

	void createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	void createMessenger();
	std::vector<QueueFamilyInfo> parseQueues();
	static std::optional<QueueID> findBestQueue(std::vector<QueueFamilyInfo>& queueFamilyInfos, vk::QueueFlags requiredFlags, float priority);
	void createLogicalDevice(const std::vector<const char*>& extensions);

	void createVmaAllocator();

	void createImmediateCommandBuffer();
	void createDefaultCommandBuffer();
};
}