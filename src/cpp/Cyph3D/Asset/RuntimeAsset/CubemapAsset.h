#pragma once

#include <Cyph3D/Asset/Processing/ImageData.h>
#include <Cyph3D/Asset/RuntimeAsset/GPUAsset.h>
#include <Cyph3D/HashBuilder.h>

#include <memory>
#include <string>

namespace c3d
{
class AssetManager;
class VKImage;
class VKSampler;

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
	~CubemapAsset() override;

	const uint32_t& getBindlessIndex() const;

private:
	friend class AssetManager;

	CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature);

	void load_async();

	std::shared_ptr<VKImage> _image;
	uint32_t _bindlessIndex;
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