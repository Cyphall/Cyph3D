#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/Asset/Processor/MeshData.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>

template<typename T>
class VKBuffer;

struct MeshAssetSignature
{
	std::string path;

	bool operator==(const MeshAssetSignature& other) const = default;
};

template<>
struct std::hash<MeshAssetSignature>
{
	std::size_t operator()(const MeshAssetSignature& key) const
	{
		return HashBuilder()
			.hash(key.path)
			.get();
	}
};

class MeshAsset : public GPUAsset<MeshAssetSignature>
{
public:
	~MeshAsset() override;

	const VKPtr<VKBuffer<VertexData>>& getVertexBuffer() const;
	const VKPtr<VKBuffer<uint32_t>>& getIndexBuffer() const;

private:
	friend class AssetManager;

	MeshAsset(AssetManager& manager, const MeshAssetSignature& signature);

	bool load_step1_mt();
	
	VKPtr<VKBuffer<VertexData>> _vertexBuffer;
	VKPtr<VKBuffer<uint32_t>> _indexBuffer;
};