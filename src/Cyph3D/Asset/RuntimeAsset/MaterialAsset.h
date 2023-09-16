#pragma once

#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/UI/IInspectable.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>
#include <memory>
#include <optional>
#include <string>

class TextureAsset;
class VKImage;
class VKImageView;

struct MaterialAssetSignature
{
	std::string path;

	bool operator==(const MaterialAssetSignature& other) const = default;
};

template<>
struct std::hash<MaterialAssetSignature>
{
	std::size_t operator()(const MaterialAssetSignature& key) const
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
	const uint32_t& getAlbedoTextureBindlessIndex() const;

	void setNormalTexture(std::optional<std::string_view> path);
	const uint32_t& getNormalTextureBindlessIndex() const;

	void setRoughnessTexture(std::optional<std::string_view> path);
	const uint32_t& getRoughnessTextureBindlessIndex() const;

	void setMetalnessTexture(std::optional<std::string_view> path);
	const uint32_t& getMetalnessTextureBindlessIndex() const;

	void setDisplacementTexture(std::optional<std::string_view> path);
	const uint32_t& getDisplacementTextureBindlessIndex() const;

	void setEmissiveTexture(std::optional<std::string_view> path);
	const uint32_t& getEmissiveTextureBindlessIndex() const;

	const glm::vec3& getAlbedoValue() const;
	void setAlbedoValue(const glm::vec3& value);

	const float& getRoughnessValue() const;
	void setRoughnessValue(const float& value);
	
	const float& getMetalnessValue() const;
	void setMetalnessValue(const float& value);
	
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
	
	void save() const;
	void reload();
	
	void uploadAlbedoValue(const glm::vec3& albedo);
	void uploadNormalValue(const glm::vec3& normal);
	void uploadRoughnessValue(const float& roughness);
	void uploadMetalnessValue(const float& metalness);
	void uploadDisplacementValue(const float& displacement);
	void uploadEmissiveValue(const float& emissive);
	
	TextureAsset* _albedoTexture = nullptr;
	sigslot::scoped_connection _albedoTextureChangedConnection;
	VKPtr<VKImage> _albedoValueTexture;
	VKPtr<VKImageView> _albedoValueTextureView;
	std::optional<uint32_t> _albedoValueTextureBindlessIndex;

	TextureAsset* _normalTexture = nullptr;
	sigslot::scoped_connection _normalTextureChangedConnection;
	VKPtr<VKImage> _normalValueTexture;
	VKPtr<VKImageView> _normalValueTextureView;
	std::optional<uint32_t> _normalValueTextureBindlessIndex;

	TextureAsset* _roughnessTexture = nullptr;
	sigslot::scoped_connection _roughnessTextureChangedConnection;
	VKPtr<VKImage> _roughnessValueTexture;
	VKPtr<VKImageView> _roughnessValueTextureView;
	std::optional<uint32_t> _roughnessValueTextureBindlessIndex;

	TextureAsset* _metalnessTexture = nullptr;
	sigslot::scoped_connection _metalnessTextureChangedConnection;
	VKPtr<VKImage> _metalnessValueTexture;
	VKPtr<VKImageView> _metalnessValueTextureView;
	std::optional<uint32_t> _metalnessValueTextureBindlessIndex;

	TextureAsset* _displacementTexture = nullptr;
	sigslot::scoped_connection _displacementTextureChangedConnection;
	VKPtr<VKImage> _displacementValueTexture;
	VKPtr<VKImageView> _displacementValueTextureView;
	std::optional<uint32_t> _displacementValueTextureBindlessIndex;

	TextureAsset* _emissiveTexture = nullptr;
	sigslot::scoped_connection _emissiveTextureChangedConnection;
	VKPtr<VKImage> _emissiveValueTexture;
	VKPtr<VKImageView> _emissiveValueTextureView;
	std::optional<uint32_t> _emissiveValueTextureBindlessIndex;

	glm::vec3 _albedoValue{};
	float _roughnessValue{};
	float _metalnessValue{};
	
	float _emissiveScale{};

	static MaterialAsset* _defaultMaterial;
	static MaterialAsset* _missingMaterial;
};