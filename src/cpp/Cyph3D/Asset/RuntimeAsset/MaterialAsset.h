#pragma once

#include <Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h>
#include <Cyph3D/HashBuilder.h>
#include <Cyph3D/UI/IInspectable.h>

#include <CyphGPU/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

namespace c3d
{
class TextureAsset;

struct MaterialAssetSignature
{
	std::string path;

	bool operator==(const MaterialAssetSignature& other) const = default;
};

class MaterialAsset : public RuntimeAsset<MaterialAssetSignature>, public IInspectable
{
public:
	~MaterialAsset() override;

	bool isLoaded() const override;

	void onDrawUi() override;

	void setAlbedoTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getAlbedoImage() const;

	void setNormalTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getNormalImage() const;

	void setRoughnessTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getRoughnessImage() const;

	void setMetalnessTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getMetalnessImage() const;

	void setDisplacementTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getDisplacementImage() const;

	void setEmissiveTexture(std::optional<std::string_view> path);
	std::optional<cgpu::ImagePtr> getEmissiveImage() const;

	const glm::vec3& getAlbedoValue() const;
	void setAlbedoValue(const glm::vec3& value);

	const float& getRoughnessValue() const;
	void setRoughnessValue(const float& value);

	const float& getMetalnessValue() const;
	void setMetalnessValue(const float& value);

	const float& getDisplacementScale() const;
	void setDisplacementScale(const float& scale);

	const float& getEmissiveScale() const;
	void setEmissiveScale(const float& scale);

	static void initDefaultAndMissing();
	static MaterialAsset* getDefaultMaterial();
	static MaterialAsset* getMissingMaterial();

	static void create(std::string_view path);

private:
	friend class AssetManager;

	MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature);

	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion3(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion4(const nlohmann::ordered_json& jsonRoot);

	void save() const;
	void reload();

	TextureAsset* _albedoTexture = nullptr;
	sigslot::scoped_connection _albedoTextureChangedConnection;

	TextureAsset* _normalTexture = nullptr;
	sigslot::scoped_connection _normalTextureChangedConnection;

	TextureAsset* _roughnessTexture = nullptr;
	sigslot::scoped_connection _roughnessTextureChangedConnection;

	TextureAsset* _metalnessTexture = nullptr;
	sigslot::scoped_connection _metalnessTextureChangedConnection;

	TextureAsset* _displacementTexture = nullptr;
	sigslot::scoped_connection _displacementTextureChangedConnection;

	TextureAsset* _emissiveTexture = nullptr;
	sigslot::scoped_connection _emissiveTextureChangedConnection;

	glm::vec3 _albedoValue{};
	float _roughnessValue{};
	float _metalnessValue{};

	float _displacementScale{};
	float _emissiveScale{};

	static MaterialAsset* _defaultMaterial;
	static MaterialAsset* _missingMaterial;
};
}

template<>
struct std::hash<c3d::MaterialAssetSignature>
{
	std::size_t operator()(const c3d::MaterialAssetSignature& key) const noexcept
	{
		return c3d::HashBuilder()
		    .hash(key.path)
		    .get();
	}
};
