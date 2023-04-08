#pragma once

#include "Cyph3D/VKObject/VKContext.h"

#include <vector>
#include <memory>

template<typename T>
class VKDynamic
{
public:
	VKDynamic() = default;
	
	VKDynamic(const VKDynamic& other) = delete;
	VKDynamic& operator=(const VKDynamic& other) = delete;
	
	VKDynamic(VKDynamic&& other) noexcept
	{
		std::swap(_objects, other._objects);
		std::swap(_context, other._context);
	}
	
	VKDynamic& operator=(VKDynamic&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		
		_objects = {};
		_context = nullptr;
		std::swap(_objects, other._objects);
		std::swap(_context, other._context);
		return *this;
	}
	
	T* operator->() const
	{
		return getVKPtr().get();
	}
	
	T* get() const
	{
		return getVKPtr().get();
	}
	
	explicit operator bool() const
	{
		return _context != nullptr;
	}
	
	const VKPtr<T>& getVKPtr() const
	{
		return _objects.empty() ? _empty : _objects[_context->getCurrentConcurrentFrame()];
	}
	
	const std::vector<VKPtr<T>>& getObjects() const
	{
		return _objects;
	}

private:
	template<typename>
	friend class VKBuffer;
	template<typename>
	friend class VKResizableBuffer;
	friend class VKCommandBuffer;
	friend class VKDescriptorSet;
	friend class VKDescriptorSetLayout;
	friend class VKFence;
	friend class VKImage;
	friend class VKImageView;
	friend class VKComputePipeline;
	friend class VKGraphicsPipeline;
	friend class VKPipelineLayout;
	friend class VKSampler;
	friend class VKSemaphore;
	friend class VKShader;
	friend class VKTimestampQuery;
	
	template<typename TArg>
	struct DynamicExtractor
	{
		TArg&& operator()(TArg&& arg, int i)
		{
			return std::forward<TArg>(arg);
		}
	};
	
	template<typename TArg>
	struct DynamicExtractor<const VKDynamic<TArg>&>
	{
		const VKPtr<TArg>& operator()(const VKDynamic<TArg>& arg, int i)
		{
			return arg.getObjects()[i];
		}
	};
	
	template<typename... TArgs>
	explicit VKDynamic(VKContext& context, TArgs&&... args):
		_context(&context)
	{
		int count = _context->getConcurrentFrameCount();
		
		_objects.reserve(count);
		for (int i = 0; i < count; i++)
		{
			_objects.push_back(T::create(context, (DynamicExtractor<TArgs>{}(std::forward<TArgs>(args), i))...));
		}
	}
	
	VKContext* _context = nullptr;
	std::vector<VKPtr<T>> _objects;
	VKPtr<T> _empty;
};