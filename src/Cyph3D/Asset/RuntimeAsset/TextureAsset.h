#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/Asset/Processor/ImageData.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <string>
#include <memory>

class VKImage;
class VKImageView;

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
	
	const VKPtr<VKImageView>& getImageView() const;

private:
	friend class AssetManager;

	TextureAsset(AssetManager& manager, const TextureAssetSignature& signature);

	bool load_step1_mt();
	
	VKPtr<VKImage> _image;
	VKPtr<VKImageView> _imageView;
};