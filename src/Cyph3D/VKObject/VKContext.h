#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>
#include <functional>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

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

	const VKPtr<VKCommandBuffer>& getDefaultCommandBuffer();

	void executeImmediate(std::function<void(const VKPtr<VKCommandBuffer>& commandBuffer)>&& function);

	const vk::PhysicalDeviceProperties& getProperties() const;
	const vk::PhysicalDeviceDescriptorIndexingProperties& getDescriptorIndexingProperties() const;
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

	vk::PhysicalDeviceProperties2 _properties;
	vk::PhysicalDeviceDescriptorIndexingProperties _descriptorIndexingProperties;
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