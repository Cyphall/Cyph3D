#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>

class Mesh;

struct ModelAssetSignature
{
	std::string path;

	bool operator==(const ModelAssetSignature& other) const = default;
};

template<>
struct std::hash<ModelAssetSignature>
{
	std::size_t operator()(const ModelAssetSignature& key) const
	{
		return HashBuilder()
			.hash(key.path)
			.get();
	}
};

class ModelAsset : public GPUAsset<ModelAssetSignature>
{
public:
	~ModelAsset() override;

	const Mesh& getMesh() const;

private:
	friend class AssetManager;

	ModelAsset(AssetManager& manager, const ModelAssetSignature& signature);

	bool load_step1_mt();

	std::unique_ptr<Mesh> _glMesh;
};