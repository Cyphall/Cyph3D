#pragma once

#include <Cyph3D/Asset/Processing/ImageData.h>
#include <Cyph3D/Asset/RuntimeAsset/GPUAsset.h>
#include <Cyph3D/HashBuilder.h>

#include <CyphGPU/fwd.hpp>
#include <memory>
#include <string>

namespace c3d
{
class AssetManager;

struct TextureAssetSignature
{
	std::string path;
	ImageType type;

	bool operator==(const TextureAssetSignature& other) const = default;
};

class TextureAsset : public GPUAsset<TextureAssetSignature>
{
public:
	const cgpu::ImagePtr& getImage() const;

private:
	friend class AssetManager;

	TextureAsset(AssetManager& manager, const TextureAssetSignature& signature);

	void load_async();

	cgpu::ImagePtr _image;
};
}

template<>
struct std::hash<c3d::TextureAssetSignature>
{
	std::size_t operator()(const c3d::TextureAssetSignature& key) const noexcept
	{
		return c3d::HashBuilder()
		    .hash(key.path)
		    .hash(key.type)
		    .get();
	}
};
