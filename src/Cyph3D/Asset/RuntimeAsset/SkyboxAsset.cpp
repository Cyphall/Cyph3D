#include "SkyboxAsset.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Asset/AssetManager.h"

SkyboxAsset::SkyboxAsset(AssetManager& manager, const SkyboxAssetSignature& signature):
	RuntimeAsset(manager, signature)
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getAssetDirectoryPath() / signature.path);

	int version;
	auto versionIt = jsonRoot.find("version");
	if (versionIt != jsonRoot.end())
	{
		version = versionIt.value().get<int>();
	}
	else
	{
		version = 1;
	}
	
	switch (version)
	{
		case 1:
			deserializeFromVersion1(jsonRoot);
			break;
		case 2:
			deserializeFromVersion2(jsonRoot);
			break;
		default:
			throw;
	}
}

SkyboxAsset::~SkyboxAsset()
{}

bool SkyboxAsset::isLoaded() const
{
	return _cubemap != nullptr && _cubemap->isLoaded();
}

const std::string& SkyboxAsset::getPath() const
{
	return _path;
}

const std::string* SkyboxAsset::getXposPath() const
{
	return _xposPath.has_value() ? &_xposPath.value() : nullptr;
}

void SkyboxAsset::setXposPath(std::optional<std::string_view> path)
{
	_xposPath = path;
	onPathChange();
}

const std::string* SkyboxAsset::getXnegPath() const
{
	return _xnegPath.has_value() ? &_xnegPath.value() : nullptr;
}

void SkyboxAsset::setXnegPath(std::optional<std::string_view> path)
{
	_xnegPath = path;
	onPathChange();
}

const std::string* SkyboxAsset::getYposPath() const
{
	return _yposPath.has_value() ? &_yposPath.value() : nullptr;
}

void SkyboxAsset::setYposPath(std::optional<std::string_view> path)
{
	_yposPath = path;
	onPathChange();
}

const std::string* SkyboxAsset::getYnegPath() const
{
	return _ynegPath.has_value() ? &_ynegPath.value() : nullptr;
}

void SkyboxAsset::setYnegPath(std::optional<std::string_view> path)
{
	_ynegPath = path;
	onPathChange();
}

const std::string* SkyboxAsset::getZposPath() const
{
	return _zposPath.has_value() ? &_zposPath.value() : nullptr;
}

void SkyboxAsset::setZposPath(std::optional<std::string_view> path)
{
	_zposPath = path;
	onPathChange();
}

const std::string* SkyboxAsset::getZnegPath() const
{
	return _znegPath.has_value() ? &_znegPath.value() : nullptr;
}

void SkyboxAsset::setZnegPath(std::optional<std::string_view> path)
{
	_znegPath = path;
	onPathChange();
}

const GLCubemap& SkyboxAsset::getCubemap() const
{
	return _cubemap->getGLCubemap();
}

void SkyboxAsset::onPathChange()
{
	if (_xposPath && _xnegPath && _yposPath && _ynegPath && _zposPath && _znegPath)
	{
		_cubemap = _manager.loadCubemap(
			_xposPath.value(),
			_xnegPath.value(),
			_ynegPath.value(),
			_yposPath.value(),
			_zposPath.value(),
			_znegPath.value()
			);
	}
	else
	{
		_cubemap = nullptr;
	}
}

void SkyboxAsset::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	setXposPath(jsonRoot["pos_x"].get<std::string>());
	setXnegPath(jsonRoot["neg_x"].get<std::string>());
	setYposPath(jsonRoot["pos_y"].get<std::string>());
	setYnegPath(jsonRoot["neg_y"].get<std::string>());
	setZposPath(jsonRoot["pos_z"].get<std::string>());
	setZnegPath(jsonRoot["neg_z"].get<std::string>());
}

void SkyboxAsset::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	const nlohmann::ordered_json& xposPath = jsonRoot["pos_x"];
	if (!xposPath.is_null())
	{
		setXposPath(xposPath.get<std::string>());
	}

	const nlohmann::ordered_json& xnegPath = jsonRoot["neg_x"];
	if (!xnegPath.is_null())
	{
		setXnegPath(xnegPath.get<std::string>());
	}

	const nlohmann::ordered_json& yposPath = jsonRoot["pos_y"];
	if (!yposPath.is_null())
	{
		setYposPath(yposPath.get<std::string>());
	}

	const nlohmann::ordered_json& ynegPath = jsonRoot["neg_y"];
	if (!ynegPath.is_null())
	{
		setYnegPath(ynegPath.get<std::string>());
	}

	const nlohmann::ordered_json& zposPath = jsonRoot["pos_z"];
	if (!zposPath.is_null())
	{
		setZposPath(zposPath.get<std::string>());
	}

	const nlohmann::ordered_json& znegPath = jsonRoot["neg_z"];
	if (!znegPath.is_null())
	{
		setZnegPath(znegPath.get<std::string>());
	}
}