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
	
	static VKDynamic<VKBuffer<T>> createDynamic(
		VKContext& context,
		size_t size,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {})
	{
		return VKDynamic<VKBuffer<T>>(context, size, pipelineUsage, requiredProperties, preferredProperties);
	}
	
	~VKBuffer() override
	{
		_context.getVmaAllocator().destroyBuffer(_buffer, _bufferAlloc);
	}
	
	T* map()
	{
		return static_cast<T*>(_context.getVmaAllocator().mapMemory(_bufferAlloc));
	}
	
	void unmap()
	{
		_context.getVmaAllocator().unmapMemory(_bufferAlloc);
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
		
		std::tie(_buffer, _bufferAlloc) = _context.getVmaAllocator().createBuffer(bufferCreateInfo, allocationCreateInfo);
		_size = size;
	}
	
	vk::Buffer _buffer;
	vma::Allocation _bufferAlloc;
	size_t _size = 0;
};