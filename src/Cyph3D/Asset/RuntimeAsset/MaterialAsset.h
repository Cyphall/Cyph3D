#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/UI/IInspectable.h"

#include <string>
#include <memory>
#include <optional>
#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

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

	const std::string& getPath() const;

	const std::string* getAlbedoMapPath() const;
	void setAlbedoMapPath(std::optional<std::string_view> path);
	const uint32_t& getAlbedoTextureBindlessIndex() const;

	const std::string* getNormalMapPath() const;
	void setNormalMapPath(std::optional<std::string_view> path);
	const uint32_t& getNormalTextureBindlessIndex() const;

	const std::string* getRoughnessMapPath() const;
	void setRoughnessMapPath(std::optional<std::string_view> path);
	const uint32_t& getRoughnessTextureBindlessIndex() const;

	const std::string* getMetalnessMapPath() const;
	void setMetalnessMapPath(std::optional<std::string_view> path);
	const uint32_t& getMetalnessTextureBindlessIndex() const;

	const std::string* getDisplacementMapPath() const;
	void setDisplacementMapPath(std::optional<std::string_view> path);
	const uint32_t& getDisplacementTextureBindlessIndex() const;

	const std::string* getEmissiveMapPath() const;
	void setEmissiveMapPath(std::optional<std::string_view> path);
	const uint32_t& getEmissiveTextureBindlessIndex() const;

	const glm::vec3& getAlbedoValue() const;
	void setAlbedoValue(const glm::vec3& value);

	const float& getRoughnessValue() const;
	void setRoughnessValue(const float& value);
	
	const float& getMetalnessValue() const;
	void setMetalnessValue(const float& value);
	
	const float& getEmissiveValue() const;
	void setEmissiveValue(const float& value);
	
	static void initDefaultAndMissing();
	static MaterialAsset* getDefaultMaterial();
	static MaterialAsset* getMissingMaterial();
	
	static void create(std::string_view path);

private:
	friend class AssetManager;

	MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature);
	
	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
	
	void save() const;
	void reload();
	
	std::optional<std::string> _albedoMapPath;
	TextureAsset* _albedoMap = nullptr;
	VKPtr<VKImage> _albedoValueTexture;
	VKPtr<VKImageView> _albedoValueTextureView;
	std::optional<uint32_t> _albedoValueTextureBindlessIndex;

	std::optional<std::string> _normalMapPath;
	TextureAsset* _normalMap = nullptr;
	VKPtr<VKImage> _normalValueTexture;
	VKPtr<VKImageView> _normalValueTextureView;
	std::optional<uint32_t> _normalValueTextureBindlessIndex;

	std::optional<std::string> _roughnessMapPath;
	TextureAsset* _roughnessMap = nullptr;
	VKPtr<VKImage> _roughnessValueTexture;
	VKPtr<VKImageView> _roughnessValueTextureView;
	std::optional<uint32_t> _roughnessValueTextureBindlessIndex;

	std::optional<std::string> _metalnessMapPath;
	TextureAsset* _metalnessMap = nullptr;
	VKPtr<VKImage> _metalnessValueTexture;
	VKPtr<VKImageView> _metalnessValueTextureView;
	std::optional<uint32_t> _metalnessValueTextureBindlessIndex;

	std::optional<std::string> _displacementMapPath;
	TextureAsset* _displacementMap = nullptr;
	VKPtr<VKImage> _displacementValueTexture;
	VKPtr<VKImageView> _displacementValueTextureView;
	std::optional<uint32_t> _displacementValueTextureBindlessIndex;

	std::optional<std::string> _emissiveMapPath;
	TextureAsset* _emissiveMap = nullptr;
	VKPtr<VKImage> _emissiveValueTexture;
	VKPtr<VKImageView> _emissiveValueTextureView;
	std::optional<uint32_t> _emissiveValueTextureBindlessIndex;

	glm::vec3 _albedoValue{};
	float _roughnessValue{};
	float _metalnessValue{};
	float _emissiveValue{};

	static MaterialAsset* _defaultMaterial;
	static MaterialAsset* _missingMaterial;
};