#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/Asset/Processor/MeshData.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>

template<typename T>
class VKBuffer;

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

	const VKPtr<VKBuffer<VertexData>>& getVertexBuffer() const;
	const VKPtr<VKBuffer<uint32_t>>& getIndexBuffer() const;

private:
	friend class AssetManager;

	ModelAsset(AssetManager& manager, const ModelAssetSignature& signature);

	bool load_step1_mt();
	
	VKPtr<VKBuffer<VertexData>> _vertexBuffer;
	VKPtr<VKBuffer<uint32_t>> _indexBuffer;
};