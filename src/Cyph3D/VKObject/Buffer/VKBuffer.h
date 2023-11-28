#pragma once

#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/Buffer/VKBufferInfo.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKContext.h"

#include <vk_mem_alloc.hpp>
#include <stdexcept>
#include <set>

template<typename T>
class VKBuffer : public VKBufferBase
{
public:
	static VKPtr<VKBuffer<T>> create(VKContext& context, const VKBufferInfo& info)
	{
		return VKPtr<VKBuffer<T>>(new VKBuffer<T>(context, info));
	}
	
	~VKBuffer() override
	{
#if defined(_DEBUG)
		if (_allocationInfo.pMappedData)
		{
			std::memset(_allocationInfo.pMappedData, 0xDD, _info.getSize() * sizeof(T));
		}
#endif
		_context.getVmaAllocator().destroyBuffer(_buffer, _allocation);
	}
	
	T* getHostPointer()
	{
		return static_cast<T*>(_allocationInfo.pMappedData);
	}
	
	const VKBufferInfo& getInfo() const
	{
		return _info;
	}
	
	const vk::Buffer& getHandle() override
	{
		return _buffer;
	}
	
	size_t getSize() const override
	{
		return _info.getSize();
	}
	
	vk::DeviceSize getByteSize() const override
	{
		return _info.getSize() * sizeof(T);
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
		VKBufferBase(context),
		_info(info)
	{
		std::set<uint32_t> queues = {
			_context.getMainQueue().getFamily(),
			_context.getComputeQueue().getFamily(),
			_context.getTransferQueue().getFamily()
		};

		std::vector<uint32_t> queuesVec(queues.begin(), queues.end());

		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.size = _info.getSize() * sizeof(T);
		bufferCreateInfo.usage = _info.getUsage();
		bufferCreateInfo.sharingMode = queuesVec.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
		bufferCreateInfo.queueFamilyIndexCount = queuesVec.size();
		bufferCreateInfo.pQueueFamilyIndices = queuesVec.data();
		
		vma::AllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = vma::MemoryUsage::eUnknown;
		allocationCreateInfo.requiredFlags = _info.getRequiredMemoryProperties();
		allocationCreateInfo.preferredFlags = _info.getPreferredMemoryProperties();
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eMapped;
		
		std::tie(_buffer, _allocation) = _context.getVmaAllocator().createBufferWithAlignment(bufferCreateInfo, allocationCreateInfo, _info.getRequiredAlignment(), _allocationInfo);
		
		if (_info.getUsage() & vk::BufferUsageFlagBits::eShaderDeviceAddress)
		{
			vk::BufferDeviceAddressInfo bufferDeviceAddressInfo;
			bufferDeviceAddressInfo.buffer = _buffer;
			
			_deviceAddress = _context.getDevice().getBufferAddress(bufferDeviceAddressInfo);
		}
		
		if (_info.hasName())
		{
			vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
			objectNameInfo.objectType = vk::ObjectType::eBuffer;
			objectNameInfo.objectHandle = reinterpret_cast<uintptr_t>(static_cast<VkBuffer>(_buffer));
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
	
	VKBufferInfo _info;
	
	vk::Buffer _buffer;
	
	vma::Allocation _allocation;
	vma::AllocationInfo _allocationInfo;
	
	vk::DeviceAddress _deviceAddress = 0;
};