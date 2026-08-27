#pragma once

#include <Cyph3D/Asset/Processing/MeshData.h>
#include <Cyph3D/Asset/RuntimeAsset/GPUAsset.h>
#include <Cyph3D/HashBuilder.h>

#include <CyphGPU/fwd.hpp>
#include <memory>
#include <string>

namespace c3d
{
class AssetManager;

struct MeshAssetSignature
{
	std::string path;

	bool operator==(const MeshAssetSignature& other) const = default;
};

class MeshAsset : public GPUAsset<MeshAssetSignature>
{
public:
	~MeshAsset() override;

	const cgpu::BufferPtr& getPositionVertexBuffer() const;
	const cgpu::BufferPtr& getMaterialVertexBuffer() const;
	const cgpu::BufferPtr& getIndexBuffer() const;
	uint32_t getIndexCount() const;
	vk::IndexType getIndexType() const;
	const cgpu::BLASPtr& getBLAS() const;

	const glm::vec3& getBoundingBoxMin() const;
	const glm::vec3& getBoundingBoxMax() const;

	static void initDefaultAndMissing();
	static MeshAsset* getDefaultMesh();
	static MeshAsset* getMissingMesh();

private:
	friend class AssetManager;

	MeshAsset(AssetManager& manager, const MeshAssetSignature& signature);

	void load_async();

	cgpu::BufferPtr _positionVertexBuffer;
	cgpu::BufferPtr _materialVertexBuffer;
	cgpu::BufferPtr _indexBuffer;
	cgpu::BLASPtr _blas;

	glm::vec3 _boundingBoxMin = {0, 0, 0};
	glm::vec3 _boundingBoxMax = {0, 0, 0};

	static MeshAsset* _defaultMesh;
	static MeshAsset* _missingMesh;
};
}

template<>
struct std::hash<c3d::MeshAssetSignature>
{
	std::size_t operator()(const c3d::MeshAssetSignature& key) const noexcept
	{
		return c3d::HashBuilder()
		    .hash(key.path)
		    .get();
	}
};
