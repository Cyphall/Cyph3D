#pragma once

#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>
#include <optional>
#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

class TextureAsset;
class GLTexture;

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

class MaterialAsset : public RuntimeAsset<MaterialAssetSignature>
{
public:
	~MaterialAsset() override;

	bool isLoaded() const override;

	const std::string& getPath() const;

	const std::string* getAlbedoMapPath() const;
	void setAlbedoMapPath(std::optional<std::string_view> path);
	const GLTexture& getAlbedoTexture() const;

	const std::string* getNormalMapPath() const;
	void setNormalMapPath(std::optional<std::string_view> path);
	const GLTexture& getNormalTexture() const;

	const std::string* getRoughnessMapPath() const;
	void setRoughnessMapPath(std::optional<std::string_view> path);
	const GLTexture& getRoughnessTexture() const;

	const std::string* getMetalnessMapPath() const;
	void setMetalnessMapPath(std::optional<std::string_view> path);
	const GLTexture& getMetalnessTexture() const;

	const std::string* getDisplacementMapPath() const;
	void setDisplacementMapPath(std::optional<std::string_view> path);
	const GLTexture& getDisplacementTexture() const;

	const std::string* getEmissiveMapPath() const;
	void setEmissiveMapPath(std::optional<std::string_view> path);
	const GLTexture& getEmissiveTexture() const;

	const glm::vec3& getAlbedoValue() const;
	void setAlbedoValue(const glm::vec3& value);

	const float& getRoughnessValue() const;
	void setRoughnessValue(const float& value);
	
	const float& getMetalnessValue() const;
	void setMetalnessValue(const float& value);
	
	const float& getEmissiveValue() const;
	void setEmissiveValue(const float& value);
	
	static void initialize();
	static MaterialAsset* getDefaultMaterial();
	static MaterialAsset* getMissingMaterial();

private:
	friend class AssetManager;

	MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature);
	
	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
	
	std::optional<std::string> _albedoMapPath;
	TextureAsset* _albedoMap = nullptr;
	std::unique_ptr<GLTexture> _albedoValueTexture;

	std::optional<std::string> _normalMapPath;
	TextureAsset* _normalMap = nullptr;
	std::unique_ptr<GLTexture> _normalValueTexture;

	std::optional<std::string> _roughnessMapPath;
	TextureAsset* _roughnessMap = nullptr;
	std::unique_ptr<GLTexture> _roughnessValueTexture;

	std::optional<std::string> _metalnessMapPath;
	TextureAsset* _metalnessMap = nullptr;
	std::unique_ptr<GLTexture> _metalnessValueTexture;

	std::optional<std::string> _displacementMapPath;
	TextureAsset* _displacementMap = nullptr;
	std::unique_ptr<GLTexture> _displacementValueTexture;

	std::optional<std::string> _emissiveMapPath;
	TextureAsset* _emissiveMap = nullptr;
	std::unique_ptr<GLTexture> _emissiveValueTexture;

	glm::vec3 _albedoValue{};
	float _roughnessValue{};
	float _metalnessValue{};
	float _emissiveValue{};

	static MaterialAsset* _defaultMaterial;
	static MaterialAsset* _missingMaterial;
};