#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <functional>
#include <filesystem>
#include <glm/glm.hpp>

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
	
	vma::Allocator getVmaAllocator();
	
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
	
	int _concurrentFrameCount;
	int _currentConcurrentFrame = 0;
	
	vk::Instance _instance;
	
	vk::DebugUtilsMessengerEXT _messenger;
	vk::PhysicalDevice _physicalDevice;
	std::unique_ptr<VKQueue> _mainQueue;
	std::unique_ptr<VKQueue> _computeQueue;
	std::unique_ptr<VKQueue> _transferQueue;
	vk::Device _device;
	
	vma::Allocator _vmaAllocator;
	
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
	bool findBestQueueFamilies(uint32_t& mainQueueFamily, uint32_t& computeQueueFamily, uint32_t& transferQueueFamily);
	void createLogicalDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
	
	void createVmaAllocator();
	
	void createImmediateCommandBuffer();
	void createDefaultCommandBuffer();
};