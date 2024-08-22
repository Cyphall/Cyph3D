#pragma once

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBufferInfo.h"
#include "Cyph3D/VKObject/VKContext.h"

template<typename T>
class VKResizableBuffer : public VKObject
{
public:
	static std::shared_ptr<VKResizableBuffer> create(VKContext& context, const VKResizableBufferInfo& info)
	{
		return std::shared_ptr<VKResizableBuffer>(new VKResizableBuffer(context, info));
	}

	T* getHostPointer()
	{
		return _buffer ? _buffer->getHostPointer() : nullptr;
	}

	const std::shared_ptr<VKBuffer<T>>& getBuffer()
	{
		return _buffer;
	}

	size_t getSize() const
	{
		return _buffer ? _buffer->getInfo().getSize() : 0;
	}

	vk::DeviceSize getByteSize() const
	{
		return _buffer ? _buffer->getByteSize() : 0;
	}

	size_t getStride() const
	{
		return _buffer ? _buffer->getStride() : 0;
	}

	vk::DeviceAddress getDeviceAddress() const
	{
		return _buffer ? _buffer->getDeviceAddress() : 0;
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
	VKResizableBuffer(VKContext& context, const VKResizableBufferInfo& info):
		VKObject(context),
		_info(info)
	{
	}

	void destroyBuffer()
	{
		_buffer = {};
	}

	void createBuffer(size_t size)
	{
		VKBufferInfo info(size, _info.getUsage());
		info.setRequiredMemoryProperties(_info.getRequiredMemoryProperties());
		info.setPreferredMemoryProperties(_info.getPreferredMemoryProperties());
		info.setRequiredAlignment(_info.getRequiredAlignment());
		if (_info.hasName())
			info.setName(_info.getName());

		_buffer = VKBuffer<T>::create(_context, info);
	}

	VKResizableBufferInfo _info;

	std::shared_ptr<VKBuffer<T>> _buffer;
};