#pragma once

#include "Cyph3D/Asset/Processing/ImageData.h"
#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"

#include <memory>
#include <string>

class AssetManager;
class VKImage;
class VKSampler;
struct AssetManagerWorkerData;

struct CubemapAssetSignature
{
	std::string xposPath;
	std::string xnegPath;
	std::string yposPath;
	std::string ynegPath;
	std::string zposPath;
	std::string znegPath;
	std::string equirectangularPath;
	ImageType type;

	bool operator==(const CubemapAssetSignature& other) const = default;
};

template<>
struct std::hash<CubemapAssetSignature>
{
	std::size_t operator()(const CubemapAssetSignature& key) const noexcept
	{
		return HashBuilder()
		    .hash(key.xposPath)
		    .hash(key.xnegPath)
		    .hash(key.yposPath)
		    .hash(key.ynegPath)
		    .hash(key.zposPath)
		    .hash(key.znegPath)
		    .hash(key.equirectangularPath)
		    .hash(key.type)
		    .get();
	}
};

class CubemapAsset : public GPUAsset<CubemapAssetSignature>
{
public:
	~CubemapAsset() override;

	const uint32_t& getBindlessIndex() const;

private:
	friend class AssetManager;

	CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature);

	void load_async(AssetManagerWorkerData& workerData);

	std::shared_ptr<VKImage> _image;
	uint32_t _bindlessIndex;
};