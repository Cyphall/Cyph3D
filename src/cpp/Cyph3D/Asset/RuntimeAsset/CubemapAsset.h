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

class CubemapAsset : public GPUAsset<CubemapAssetSignature>
{
public:
	const cgpu::ImagePtr& getImage() const;

private:
	friend class AssetManager;

	CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature);

	void load_async();

	cgpu::ImagePtr _image;
};
}

template<>
struct std::hash<c3d::CubemapAssetSignature>
{
	std::size_t operator()(const c3d::CubemapAssetSignature& key) const noexcept
	{
		return c3d::HashBuilder()
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
