#pragma once

#include "Cyph3D/Asset/Processing/MeshData.h"
#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/HashBuilder.h"

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
	std::size_t operator()(const MeshAssetSignature& key) const noexcept
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

	const std::shared_ptr<VKBuffer<PositionVertexData>>& getPositionVertexBuffer() const;
	const std::shared_ptr<VKBuffer<MaterialVertexData>>& getMaterialVertexBuffer() const;
	const std::shared_ptr<VKBuffer<uint32_t>>& getIndexBuffer() const;
	const std::shared_ptr<VKAccelerationStructure>& getAccelerationStructure() const;

	const glm::vec3& getBoundingBoxMin() const;
	const glm::vec3& getBoundingBoxMax() const;

	static void initDefaultAndMissing();
	static MeshAsset* getDefaultMesh();
	static MeshAsset* getMissingMesh();

private:
	friend class AssetManager;

	MeshAsset(AssetManager& manager, const MeshAssetSignature& signature);

	void load_async(AssetManagerWorkerData& workerData);

	std::shared_ptr<VKBuffer<PositionVertexData>> _positionVertexBuffer;
	std::shared_ptr<VKBuffer<MaterialVertexData>> _materialVertexBuffer;
	std::shared_ptr<VKBuffer<uint32_t>> _indexBuffer;
	std::shared_ptr<VKAccelerationStructure> _accelerationStructure;

	glm::vec3 _boundingBoxMin = {0, 0, 0};
	glm::vec3 _boundingBoxMax = {0, 0, 0};

	static MeshAsset* _defaultMesh;
	static MeshAsset* _missingMesh;
};