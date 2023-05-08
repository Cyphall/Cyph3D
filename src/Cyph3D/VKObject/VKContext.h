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
	VKQueue& getQueue();
	
	vma::Allocator getVmaAllocator();
	
	const VKPtr<VKCommandBuffer>& getDefaultCommandBuffer();
	
	void executeImmediate(std::function<void(const VKPtr<VKCommandBuffer>& commandBuffer)>&& function);
	
	const vk::PhysicalDeviceProperties& getProperties() const;
	const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& getRayTracingPipelineProperties() const;

private:
	struct HelperData;
	
	int _concurrentFrameCount;
	int _currentConcurrentFrame = 0;
	
	std::vector<const char*> _instanceExtensions;
	std::vector<const char*> _layers;
	std::vector<const char*> _deviceExtensions;
	
	vk::Instance _instance;
	
	vk::DebugUtilsMessengerEXT _messenger;
	vk::PhysicalDevice _physicalDevice;
	std::unique_ptr<VKQueue> _queue;
	vk::Device _device;
	
	vma::Allocator _vmaAllocator;
	
	std::unique_ptr<HelperData> _helperData;
	
	vk::PhysicalDeviceProperties2 _properties;
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR _rayTracingPipelineProperties;
	
	explicit VKContext(int concurrentFrameCount);
	
	int calculateDeviceScore(const vk::PhysicalDevice& device) const;
	bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
	
	void fillInstanceExtensions();
	void fillLayers();
	void fillDeviceExtensions();
	
	void checkInstanceExtensionSupport();
	void checkLayerSupport();
	
	void createInstance();
	
	void createMessenger();
	void selectPhysicalDevice();
	uint32_t findSuitableQueueFamily();
	void createLogicalDevice();
	
	void createVmaAllocator();
	
	void createImmediateCommandBuffer();
	void createDefaultCommandBuffer();
};