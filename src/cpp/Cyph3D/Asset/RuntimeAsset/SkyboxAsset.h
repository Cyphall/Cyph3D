#pragma once

#include "Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
#include "Cyph3D/HashBuilder.h"
#include "Cyph3D/UI/IInspectable.h"

#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

class CubemapAsset;
class TextureAsset;
class VKImage;

struct SkyboxAssetSignature
{
	std::string path;

	bool operator==(const SkyboxAssetSignature& other) const = default;
};

template<>
struct std::hash<SkyboxAssetSignature>
{
	std::size_t operator()(const SkyboxAssetSignature& key) const noexcept
	{
		return HashBuilder()
		    .hash(key.path)
		    .get();
	}
};

class SkyboxAsset : public RuntimeAsset<SkyboxAssetSignature>, public IInspectable
{
public:
	enum class Layout
	{
		Cubemap,
		Equirectangular
	};

	~SkyboxAsset() override;

	bool isLoaded() const override;

	void onDrawUi() override;

	const Layout& getLayout() const;
	void setLayout(const Layout& layout);

	const std::string* getXposPath() const;
	void setXposPath(std::optional<std::string_view> path);

	const std::string* getXnegPath() const;
	void setXnegPath(std::optional<std::string_view> path);

	const std::string* getYposPath() const;
	void setYposPath(std::optional<std::string_view> path);

	const std::string* getYnegPath() const;
	void setYnegPath(std::optional<std::string_view> path);

	const std::string* getZposPath() const;
	void setZposPath(std::optional<std::string_view> path);

	const std::string* getZnegPath() const;
	void setZnegPath(std::optional<std::string_view> path);

	const std::string* getEquirectangularPath() const;
	void setEquirectangularPath(std::optional<std::string_view> path);

	CubemapAsset* getCubemap() const;

	static void create(std::string_view path);

private:
	friend class AssetManager;

	SkyboxAsset(AssetManager& manager, const SkyboxAssetSignature& signature);

	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion3(const nlohmann::ordered_json& jsonRoot);

	void save() const;
	void reload();

	void onChanged();

	Layout _layout = Layout::Cubemap;

	std::optional<std::string> _xposPath;
	std::optional<std::string> _xnegPath;
	std::optional<std::string> _yposPath;
	std::optional<std::string> _ynegPath;
	std::optional<std::string> _zposPath;
	std::optional<std::string> _znegPath;

	std::optional<std::string> _equirectangularPath;

	CubemapAsset* _cubemap = nullptr;
	sigslot::scoped_connection _cubemapChangedConnection;
};