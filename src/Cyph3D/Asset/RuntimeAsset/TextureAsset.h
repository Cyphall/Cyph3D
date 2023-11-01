#pragma once

#include "Cyph3D/Asset/Processing/ImageData.h"
#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <memory>
#include <string>

class AssetManager;
class VKImage;
class VKSampler;
struct AssetManagerWorkerData;

struct TextureAssetSignature
{
	std::string path;
	ImageType type;

	bool operator==(const TextureAssetSignature& other) const = default;
};

template<>
struct std::hash<TextureAssetSignature>
{
	std::size_t operator()(const TextureAssetSignature& key) const
	{
		return HashBuilder()
			.hash(key.path)
			.hash(key.type)
			.get();
	}
};

class TextureAsset : public GPUAsset<TextureAssetSignature>
{
public:
	~TextureAsset() override;
	
	const uint32_t& getBindlessIndex() const;

private:
	friend class AssetManager;

	TextureAsset(AssetManager& manager, const TextureAssetSignature& signature);
	
	void load_async(AssetManagerWorkerData& workerData);
	
	VKPtr<VKImage> _image;
	uint32_t _bindlessIndex;
};