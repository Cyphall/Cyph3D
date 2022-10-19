#pragma once

#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"

#include <atomic>

template<typename TSignature>
class GPUAsset : public RuntimeAsset<TSignature>
{
public:
	using RuntimeAsset<TSignature>::RuntimeAsset;

	bool isLoaded() const override
	{
		return _loaded;
	}
	
protected:
	std::atomic_bool _loaded = false;
};