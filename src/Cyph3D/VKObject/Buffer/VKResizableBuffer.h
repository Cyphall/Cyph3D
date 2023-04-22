#pragma once

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/VKContext.h"

template<typename T>
class VKResizableBuffer : public VKObject
{
public:
	static VKPtr<VKResizableBuffer<T>> create(
		VKContext& context,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {})
	{
		return VKPtr<VKResizableBuffer<T>>(new VKResizableBuffer<T>(context, pipelineUsage, requiredProperties, preferredProperties));
	}
	
	static VKDynamic<VKResizableBuffer<T>> createDynamic(
		VKContext& context,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties = {},
		vk::MemoryPropertyFlags preferredProperties = {})
	{
		return VKDynamic<VKResizableBuffer<T>>(context, pipelineUsage, requiredProperties, preferredProperties);
	}
	
	T* map()
	{
		return _buffer ? _buffer->map() : nullptr;
	}
	
	void unmap()
	{
		if (_buffer)
		{
			_buffer->unmap();
		}
	}
	
	const VKPtr<VKBuffer<T>>& getBuffer()
	{
		return _buffer;
	}
	
	size_t getSize() const
	{
		return _buffer ? _buffer->getSize() : 0;
	}
	
	vk::DeviceSize getByteSize() const
	{
		return _buffer ? _buffer->getByteSize() : 0;
	}
	
	size_t getStride() const
	{
		return _buffer ? _buffer->getStride() : 0;
	}
	
	void resize(size_t newSize)
	{
		size_t currentSize = getSize();
		
		if (newSize == currentSize)
		{
			return;
		}
		
		if (_buffer)
		{
			destroyBuffer();
		}
		
		if (newSize > 0)
		{
			createBuffer(newSize);
		}
	}
	
	void resizeSmart(size_t minRequestedSize)
	{
		size_t currentSize = getSize();
		
		if (minRequestedSize <= currentSize)
		{
			return;
		}
		
		// resize to next power of 2
		size_t tentativeSize = 1;
		while (tentativeSize < minRequestedSize)
		{
			tentativeSize *= 2;
		}
		
		resize(tentativeSize);
	}
	
private:
	VKResizableBuffer(
		VKContext& context,
		vk::BufferUsageFlags pipelineUsage,
		vk::MemoryPropertyFlags requiredProperties,
		vk::MemoryPropertyFlags preferredProperties):
		VKObject(context),
		_pipelineUsage(pipelineUsage),
		_requiredProperties(requiredProperties),
		_preferredProperties(preferredProperties)
	{
	
	}
	
	void destroyBuffer()
	{
		_buffer = {};
	}
	
	void createBuffer(size_t size)
	{
		_buffer = VKBuffer<T>::create(_context, size, _pipelineUsage, _requiredProperties, _preferredProperties);
	}
	
	vk::BufferUsageFlags _pipelineUsage;
	vk::MemoryPropertyFlags _requiredProperties;
	vk::MemoryPropertyFlags _preferredProperties;
	
	VKPtr<VKBuffer<T>> _buffer;
};