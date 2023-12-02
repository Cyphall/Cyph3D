#pragma once

#include "Cyph3D/Asset/Processing/MeshData.h"
#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <memory>
#include <string>

class AssetManager;
template<typename T>
class VKBuffer;
class VKAccelerationStructure;
struct AssetManagerWorkerData;

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

	const VKPtr<VKBuffer<PositionVertexData>>& getPositionVertexBuffer() const;
	const VKPtr<VKBuffer<FullVertexData>>& getFullVertexBuffer() const;
	const VKPtr<VKBuffer<uint32_t>>& getIndexBuffer() const;
	const VKPtr<VKAccelerationStructure>& getAccelerationStructure() const;

	const glm::vec3& getBoundingBoxMin() const;
	const glm::vec3& getBoundingBoxMax() const;

	static void initDefaultAndMissing();
	static MeshAsset* getDefaultMesh();
	static MeshAsset* getMissingMesh();

private:
	friend class AssetManager;

	MeshAsset(AssetManager& manager, const MeshAssetSignature& signature);

	void load_async(AssetManagerWorkerData& workerData);

	VKPtr<VKBuffer<PositionVertexData>> _positionVertexBuffer;
	VKPtr<VKBuffer<FullVertexData>> _fullVertexBuffer;
	VKPtr<VKBuffer<uint32_t>> _indexBuffer;
	VKPtr<VKAccelerationStructure> _accelerationStructure;

	glm::vec3 _boundingBoxMin;
	glm::vec3 _boundingBoxMax;

	static MeshAsset* _defaultMesh;
	static MeshAsset* _missingMesh;
};