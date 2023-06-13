#pragma once

#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/Asset/Processing/ImageData.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <string>
#include <memory>

class AssetManager;
class VKImage;
class VKImageView;
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
	ImageType type;

	bool operator==(const CubemapAssetSignature& other) const = default;
};

template<>
struct std::hash<CubemapAssetSignature>
{
	std::size_t operator()(const CubemapAssetSignature& key) const
	{
		return HashBuilder()
			.hash(key.xposPath)
			.hash(key.xnegPath)
			.hash(key.yposPath)
			.hash(key.ynegPath)
			.hash(key.zposPath)
			.hash(key.znegPath)
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
	
	VKPtr<VKImage> _image;
	VKPtr<VKImageView> _imageView;
	uint32_t _bindlessIndex;
};