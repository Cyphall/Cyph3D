#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <functional>
#include <filesystem>
#include <glm/glm.hpp>
#include <unordered_set>

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
	VKQueue& getQueue();
	
	vma::Allocator getVmaAllocator();
	
	const VKPtr<VKCommandBuffer>& getDefaultCommandBuffer();
	
	void executeImmediate(std::function<void(const VKPtr<VKCommandBuffer>& commandBuffer)>&& function);
	
	const vk::PhysicalDeviceProperties& getProperties() const;
	const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& getRayTracingPipelineProperties() const;
	const vk::PhysicalDeviceDescriptorIndexingProperties& getDescriptorIndexingProperties() const;
	
	bool isRayTracingSupported() const;

private:
	struct HelperData;
	
	int _concurrentFrameCount;
	int _currentConcurrentFrame = 0;
	
	vk::Instance _instance;
	
	vk::DebugUtilsMessengerEXT _messenger;
	vk::PhysicalDevice _physicalDevice;
	std::unique_ptr<VKQueue> _queue;
	vk::Device _device;
	
	vma::Allocator _vmaAllocator;
	
	std::unique_ptr<HelperData> _helperData;
	
	vk::PhysicalDeviceProperties2 _properties;
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR _rayTracingPipelineProperties;
	vk::PhysicalDeviceDescriptorIndexingProperties _descriptorIndexingProperties;
	
	bool _rayTracingSupported;
	
	explicit VKContext(int concurrentFrameCount);
	
	void createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
	
	void createMessenger();
	uint32_t findSuitableQueueFamily();
	void createLogicalDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
	
	void createVmaAllocator();
	
	void createImmediateCommandBuffer();
	void createDefaultCommandBuffer();
};