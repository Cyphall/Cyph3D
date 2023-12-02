#pragma once

#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/UI/IInspectable.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

class TextureAsset;
class VKImage;

struct MaterialAssetSignature
{
	std::string path;

	bool operator==(const MaterialAssetSignature& other) const = default;
};

template<>
struct std::hash<MaterialAssetSignature>
{
	std::size_t operator()(const MaterialAssetSignature& key) const noexcept
	{
		return HashBuilder()
		    .hash(key.path)
		    .get();
	}
};

class MaterialAsset : public RuntimeAsset<MaterialAssetSignature>, public IInspectable
{
public:
	~MaterialAsset() override;

	bool isLoaded() const override;

	void onDrawUi() override;

	void setAlbedoTexture(std::optional<std::string_view> path);
	int32_t getAlbedoTextureBindlessIndex() const;

	void setNormalTexture(std::optional<std::string_view> path);
	int32_t getNormalTextureBindlessIndex() const;

	void setRoughnessTexture(std::optional<std::string_view> path);
	int32_t getRoughnessTextureBindlessIndex() const;

	void setMetalnessTexture(std::optional<std::string_view> path);
	int32_t getMetalnessTextureBindlessIndex() const;

	void setDisplacementTexture(std::optional<std::string_view> path);
	int32_t getDisplacementTextureBindlessIndex() const;

	void setEmissiveTexture(std::optional<std::string_view> path);
	int32_t getEmissiveTextureBindlessIndex() const;

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