#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>
#include <array>

class GLCubemap;

struct CubemapAssetSignature
{
	std::string xposPath;
	std::string xnegPath;
	std::string yposPath;
	std::string ynegPath;
	std::string zposPath;
	std::string znegPath;

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
			.get();
	}
};

class CubemapAsset : public GPUAsset<CubemapAssetSignature>
{
public:
	~CubemapAsset() override;

	const GLCubemap& getGLCubemap() const;

private:
	friend class AssetManager;

	CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature);

	bool load_step1_mt();

	std::unique_ptr<GLCubemap> _glCubemap;
};