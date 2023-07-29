#pragma once

#include <stdexcept>
#include <sigslot/signal.hpp>

class AssetManager;

template<typename TSignature>
class RuntimeAsset
{
public:
	virtual ~RuntimeAsset() = default;

	RuntimeAsset(const RuntimeAsset& other) = delete;
	RuntimeAsset& operator=(const RuntimeAsset& other) = delete;

	RuntimeAsset(RuntimeAsset&& other) = delete;
	RuntimeAsset& operator=(RuntimeAsset&& other) = delete;

	virtual bool isLoaded() const = 0;
	
	const TSignature& getSignature() const
	{
		return _signature;
	}
	
	sigslot::signal<>& getChangedSignal()
	{
		return _changed;
	}

protected:
	explicit RuntimeAsset(AssetManager& manager, const TSignature& signature):
		_manager(manager), _signature(signature)
	{

	}
	
	void checkLoaded() const
	{
#if defined(_DEBUG)
		if (!isLoaded())
		{
			throw std::runtime_error("Attempted to access an unloaded resource.");
		}
#endif
	}
	
	AssetManager& _manager;
	TSignature _signature;
	
	sigslot::signal<> _changed;
};