#pragma once

#include "Cyph3D/VKObject/VKContext.h"

#include <memory>
#include <vector>

template<typename T>
class VKDynamic
{
public:
	VKDynamic() = default;

	explicit VKDynamic(VKContext& context, std::function<std::shared_ptr<T>(VKContext&, int)> createCallback):
		_context(&context)
	{
		int count = _context->getConcurrentFrameCount();

		_objects.reserve(count);
		for (int i = 0; i < count; i++)
		{
			_objects.push_back(createCallback(*_context, i));
		}
	}

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
		return getCurrent().get();
	}

	explicit operator bool() const
	{
		return _context != nullptr;
	}

	const std::shared_ptr<T>& getCurrent() const
	{
		return _objects.empty() ? _empty : _objects[_context->getCurrentConcurrentFrame()];
	}

	const std::shared_ptr<T>& operator[](size_t index) const
	{
		return _objects[index];
	}

private:
	VKContext* _context = nullptr;
	std::vector<std::shared_ptr<T>> _objects;
	std::shared_ptr<T> _empty;
};