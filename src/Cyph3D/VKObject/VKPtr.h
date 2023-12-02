#pragma once

#include <type_traits>

class VKPtrBase
{
public:
	virtual ~VKPtrBase() = default;

protected:
	VKPtrBase() = default;

	class ControlBlockBase
	{
	public:
		virtual ~ControlBlockBase() = default;

		void increment()
		{
			_refCount++;
		}

		void decrement()
		{
			if (--_refCount == 0)
			{
				delete this;
			}
		}

		uint32_t getRefCount() const
		{
			return _refCount;
		}

	private:
		uint32_t _refCount = 1;
	};
};

template<typename T>
class VKPtr : private VKPtrBase
{
public:
	VKPtr() = default;

	~VKPtr() override
	{
		release();
	}

	explicit VKPtr(T* pointer):
		_pointer(pointer),
		_controlBlock(new ControlBlock(pointer))
	{}

	VKPtr(const VKPtr& other)
	{
		copyConstructFrom(other);
	}

	template<typename TOther, std::enable_if_t<std::is_convertible_v<TOther*, T*>>* = nullptr>
	VKPtr(const VKPtr<TOther>& other) // NOLINT(google-explicit-constructor)
	{
		copyConstructFrom(other);
	}

	VKPtr& operator=(const VKPtr& other)
	{
		if (this == &other)
		{
			return *this;
		}

		return copyAssignFrom(other); // NOLINT(misc-unconventional-assign-operator)
	}

	template<typename TOther, std::enable_if_t<std::is_convertible_v<TOther*, T*>>* = nullptr>
	VKPtr& operator=(const VKPtr<TOther>& other)
	{
		return copyAssignFrom(other); // NOLINT(misc-unconventional-assign-operator)
	}

	VKPtr(VKPtr&& other) noexcept
	{
		moveConstructFrom(std::forward<VKPtr>(other));
	}

	template<typename TOther, std::enable_if_t<std::is_convertible_v<TOther*, T*>>* = nullptr>
	VKPtr(VKPtr<TOther>&& other) noexcept // NOLINT(google-explicit-constructor)
	{
		moveConstructFrom(std::forward<VKPtr<TOther>>(other));
	}

	VKPtr& operator=(VKPtr&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		return moveAssignFrom(std::forward<VKPtr>(other)); // NOLINT(misc-unconventional-assign-operator)
	}

	template<typename TOther, std::enable_if_t<std::is_convertible_v<TOther*, T*>>* = nullptr>
	VKPtr& operator=(VKPtr<TOther>&& other) noexcept
	{
		return moveAssignFrom(std::forward<VKPtr<TOther>>(other)); // NOLINT(misc-unconventional-assign-operator)
	}

	T* operator->() const
	{
		return get();
	}

	T& operator*() const
	{
		return *get();
	}

	T* get() const
	{
		return _pointer;
	}

	explicit operator bool() const
	{
		return _controlBlock != nullptr;
	}

	uint32_t getRefCount() const
	{
		return _controlBlock != nullptr ? _controlBlock->getRefCount() : 0;
	}

private:
	template<typename>
	friend class VKPtr;

	class ControlBlock : public ControlBlockBase
	{
	public:
		explicit ControlBlock(T* pointer):
			_pointer(pointer)
		{
		}

		~ControlBlock() override
		{
			delete _pointer;
		}

	private:
		T* _pointer;
	};

	void acquire(T* pointer, ControlBlockBase* controlBlock)
	{
		_pointer = pointer;
		_controlBlock = controlBlock;
		if (_controlBlock != nullptr)
		{
			_controlBlock->increment();
		}
	}

	void release()
	{
		if (_controlBlock)
		{
			_controlBlock->decrement();
		}
		_controlBlock = nullptr;
		_pointer = nullptr;
	}

	template<typename TOther>
	void copyConstructFrom(const VKPtr<TOther>& other)
	{
		acquire(other._pointer, other._controlBlock);
	}

	template<typename TOther>
	VKPtr& copyAssignFrom(const VKPtr<TOther>& other)
	{
		release();
		copyConstructFrom(std::forward<const VKPtr<TOther>>(other));
		return *this;
	}

	template<typename TOther>
	void moveConstructFrom(VKPtr<TOther>&& other)
	{
		_pointer = other._pointer;
		_controlBlock = other._controlBlock;
		other._pointer = nullptr;
		other._controlBlock = nullptr;
	}

	template<typename TOther>
	VKPtr& moveAssignFrom(VKPtr<TOther>&& other)
	{
		release();
		moveConstructFrom(std::forward<VKPtr<TOther>>(other));
		return *this;
	}

	T* _pointer = nullptr;
	ControlBlockBase* _controlBlock = nullptr;
};