#pragma once

#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/VKContext.h"

#include <stdexcept>
#include <vk_mem_alloc.hpp>

template<typename T>
class VKBuffer : public VKBufferBase
{
public:
	static VKPtr<VKBuffer<T>> create(
		VKContext& context,
		size_t size,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {})
	{
		return VKPtr<VKBuffer<T>>(new VKBuffer<T>(context, size, pipelineUsage, requiredProperties, preferredProperties));
	}
	
	~VKBuffer() override
	{
		_context.getVmaAllocator().destroyBuffer(_buffer, _allocation);
	}
	
	T* getHostPointer()
	{
		return static_cast<T*>(_allocationInfo.pMappedData);
	}
	
	const vk::Buffer& getHandle() override
	{
		return _buffer;
	}
	
	size_t getSize() const override
	{
		return _size;
	}
	
	vk::DeviceSize getByteSize() const override
	{
		return _size * sizeof(T);
	}
	
	size_t getStride() const override
	{
		return sizeof(T);
	}
	
	vk::DeviceAddress getDeviceAddress() const override
	{
		if (_deviceAddress == 0)
		{
			throw;
		}
		
		return _deviceAddress;
	}

private:
	VKBuffer(
		VKContext& context,
		size_t size,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties,
		vk::MemoryPropertyFlags preferredProperties):
		VKBufferBase(context)
	{
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.size = size * sizeof(T);
		bufferCreateInfo.usage = pipelineUsage;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		
		vma::AllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = vma::MemoryUsage::eUnknown;
		allocationCreateInfo.requiredFlags = requiredProperties;
		allocationCreateInfo.preferredFlags = preferredProperties;
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eMapped;
		
		std::tie(_buffer, _allocation) = _context.getVmaAllocator().createBuffer(bufferCreateInfo, allocationCreateInfo, _allocationInfo);
		_size = size;
		
		if (pipelineUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
		{
			vk::BufferDeviceAddressInfo info;
			info.buffer = _buffer;
			
			_deviceAddress = _context.getDevice().getBufferAddress(info);
		}
	}
	
	vk::Buffer _buffer;
	vma::Allocation _allocation;
	vma::AllocationInfo _allocationInfo;
	size_t _size = 0;
	vk::DeviceAddress _deviceAddress = 0;
};