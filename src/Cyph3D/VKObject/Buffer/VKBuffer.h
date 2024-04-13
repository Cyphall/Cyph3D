#pragma once

#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/VKContext.h"

#include <vk_mem_alloc.h>

template<typename T>
class VKBuffer : public VKBufferBase
{
public:
	static VKPtr<VKBuffer> create(VKContext& context, const VKBufferInfo& info)
	{
		return VKPtr<VKBuffer>(new VKBuffer(context, info));
	}

	~VKBuffer() override
	{
#if defined(_DEBUG)
		if (_allocationInfo.pMappedData)
		{
			std::memset(_allocationInfo.pMappedData, 0xDD, getByteSize());
		}
#endif
		vmaDestroyBuffer(_context.getVmaAllocator(), _handle, _allocation);
	}

	T* getHostPointer()
	{
		return static_cast<T*>(_allocationInfo.pMappedData);
	}

	const vk::Buffer& getHandle() override
	{
		return _handle;
	}

	size_t getStride() const override
	{
		return sizeof(T);
	}

	vk::DeviceAddress getDeviceAddress() const override
	{
		return _deviceAddress;
	}

private:
	VKBuffer(VKContext& context, const VKBufferInfo& info):
		VKBufferBase(context, info)
	{
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.size = _info.getSize() * sizeof(T);
		bufferCreateInfo.usage = _info.getUsage();
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
		allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(_info.getRequiredMemoryProperties());
		allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(_info.getPreferredMemoryProperties());
		allocationCreateInfo.memoryTypeBits = std::numeric_limits<uint32_t>::max();
		allocationCreateInfo.pool = nullptr;
		allocationCreateInfo.pUserData = nullptr;
		allocationCreateInfo.priority = 0.5f;

		vmaCreateBufferWithAlignment(
			_context.getVmaAllocator(),
			reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo),
			&allocationCreateInfo,
			_info.getRequiredAlignment(),
			reinterpret_cast<VkBuffer*>(&_handle),
			&_allocation,
			&_allocationInfo
		);

		if (_info.getUsage() & vk::BufferUsageFlagBits::eShaderDeviceAddress)
		{
			vk::BufferDeviceAddressInfo bufferDeviceAddressInfo;
			bufferDeviceAddressInfo.buffer = _handle;

			_deviceAddress = _context.getDevice().getBufferAddress(bufferDeviceAddressInfo);
		}

		if (_info.hasName())
		{
			vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
			objectNameInfo.objectType = vk::ObjectType::eBuffer;
			objectNameInfo.objectHandle = reinterpret_cast<uintptr_t>(static_cast<VkBuffer>(_handle));
			objectNameInfo.pObjectName = _info.getName().c_str();

			_context.getDevice().setDebugUtilsObjectNameEXT(objectNameInfo);
		}

#if defined(_DEBUG)
		if (_allocationInfo.pMappedData)
		{
			std::memset(_allocationInfo.pMappedData, 0xCD, _info.getSize() * sizeof(T));
		}
#endif
	}

	vk::Buffer _handle;
	VmaAllocation _allocation = VK_NULL_HANDLE;
	VmaAllocationInfo _allocationInfo;

	vk::DeviceAddress _deviceAddress = 0;
};