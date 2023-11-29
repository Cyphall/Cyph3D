#include "SkyboxAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/ImGuiHelper.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Window.h"

#include <magic_enum.hpp>

SkyboxAsset::SkyboxAsset(AssetManager& manager, const SkyboxAssetSignature& signature):
	RuntimeAsset(manager, signature)
{
	reload();
}

SkyboxAsset::~SkyboxAsset()
{}

bool SkyboxAsset::isLoaded() const
{
	return _cubemap != nullptr && _cubemap->isLoaded();
}

void SkyboxAsset::onDrawUi()
{
	ImGuiHelper::TextCentered("Skybox");
	ImGuiHelper::TextCentered(_signature.path.c_str());

	ImGui::Separator();

	if (ImGui::Button("Reload"))
	{
		reload();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		save();
	}
	
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	
	if (ImGui::BeginCombo("Layout", magic_enum::enum_name(_layout).data()))
	{
		for (Layout layout : magic_enum::enum_values<Layout>())
		{
			bool isSelected = (layout == _layout);
			if (ImGui::Selectable(magic_enum::enum_name(layout).data(), isSelected))
			{
				setLayout(layout);
			}
			
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	switch (_layout)
	{
		case Layout::Cubemap:
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Positive X");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getXposPath(), "Image", "asset_image", newPath))
				{
					setXposPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Negative X");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getXnegPath(), "Image", "asset_image", newPath))
				{
					setXnegPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Positive Y");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getYposPath(), "Image", "asset_image", newPath))
				{
					setYposPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Negative Y");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getYnegPath(), "Image", "asset_image", newPath))
				{
					setYnegPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Positive Z");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getZposPath(), "Image", "asset_image", newPath))
				{
					setZposPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Negative Z");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getZnegPath(), "Image", "asset_image", newPath))
				{
					setZnegPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			break;
		case Layout::Equirectangular:
			ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
			{
				ImGuiHelper::BeginGroupPanel("Equirectangular");
				
				std::optional<std::string_view> newPath;
				if (ImGuiHelper::AssetInputWidget(getEquirectangularPath(), "Image", "asset_image", newPath))
				{
					setEquirectangularPath(newPath);
				}
				
				ImGuiHelper::EndGroupPanel();
			}
			break;
		default:
			throw;
	}
}

const SkyboxAsset::Layout& SkyboxAsset::getLayout() const
{
	return _layout;
}

void SkyboxAsset::setLayout(const SkyboxAsset::Layout& layout)
{
	_layout = layout;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getXposPath() const
{
	return _xposPath.has_value() ? &_xposPath.value() : nullptr;
}

void SkyboxAsset::setXposPath(std::optional<std::string_view> path)
{
	_xposPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getXnegPath() const
{
	return _xnegPath.has_value() ? &_xnegPath.value() : nullptr;
}

void SkyboxAsset::setXnegPath(std::optional<std::string_view> path)
{
	_xnegPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getYposPath() const
{
	return _yposPath.has_value() ? &_yposPath.value() : nullptr;
}

void SkyboxAsset::setYposPath(std::optional<std::string_view> path)
{
	_yposPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getYnegPath() const
{
	return _ynegPath.has_value() ? &_ynegPath.value() : nullptr;
}

void SkyboxAsset::setYnegPath(std::optional<std::string_view> path)
{
	_ynegPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getZposPath() const
{
	return _zposPath.has_value() ? &_zposPath.value() : nullptr;
}

void SkyboxAsset::setZposPath(std::optional<std::string_view> path)
{
	_zposPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getZnegPath() const
{
	return _znegPath.has_value() ? &_znegPath.value() : nullptr;
}

void SkyboxAsset::setZnegPath(std::optional<std::string_view> path)
{
	_znegPath = path;
	onChanged();
	
	_changed();
}

const std::string* SkyboxAsset::getEquirectangularPath() const
{
	return _equirectangularPath.has_value() ? &_equirectangularPath.value() : nullptr;
}

void SkyboxAsset::setEquirectangularPath(std::optional<std::string_view> path)
{
	_equirectangularPath = path;
	onChanged();
	
	_changed();
}

CubemapAsset* SkyboxAsset::getCubemap() const
{
	return _cubemap;
}

void SkyboxAsset::create(std::string_view path)
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 2;
	
	jsonRoot["pos_x"] = nullptr;
	jsonRoot["neg_x"] = nullptr;
	jsonRoot["pos_y"] = nullptr;
	jsonRoot["neg_y"] = nullptr;
	jsonRoot["pos_z"] = nullptr;
	jsonRoot["neg_z"] = nullptr;

	JsonHelper::saveJsonToFile(jsonRoot, FileHelper::getAssetDirectoryPath() / path);
}

void SkyboxAsset::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	setLayout(Layout::Cubemap);
	setXposPath(jsonRoot["pos_x"].get<std::string>());
	setXnegPath(jsonRoot["neg_x"].get<std::string>());
	setYposPath(jsonRoot["pos_y"].get<std::string>());
	setYnegPath(jsonRoot["neg_y"].get<std::string>());
	setZposPath(jsonRoot["pos_z"].get<std::string>());
	setZnegPath(jsonRoot["neg_z"].get<std::string>());
}

void SkyboxAsset::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	setLayout(Layout::Cubemap);
	
	{
		const nlohmann::ordered_json& path = jsonRoot["pos_x"];
		if (!path.is_null())
		{
			setXposPath(path.get<std::string>());
		}
		else
		{
			setXposPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& path = jsonRoot["neg_x"];
		if (!path.is_null())
		{
			setXnegPath(path.get<std::string>());
		}
		else
		{
			setXnegPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& path = jsonRoot["pos_y"];
		if (!path.is_null())
		{
			setYposPath(path.get<std::string>());
		}
		else
		{
			setYposPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& path = jsonRoot["neg_y"];
		if (!path.is_null())
		{
			setYnegPath(path.get<std::string>());
		}
		else
		{
			setYnegPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& path = jsonRoot["pos_z"];
		if (!path.is_null())
		{
			setZposPath(path.get<std::string>());
		}
		else
		{
			setZposPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& path = jsonRoot["neg_z"];
		if (!path.is_null())
		{
			setZnegPath(path.get<std::string>());
		}
		else
		{
			setZnegPath(std::nullopt);
		}
	}
}

void SkyboxAsset::deserializeFromVersion3(const nlohmann::ordered_json& jsonRoot)
{
	{
		const nlohmann::ordered_json& layout = jsonRoot["layout"];
		if (layout == "cubemap")
		{
			setLayout(Layout::Cubemap);
		}
		else if (layout == "equirectangular")
		{
			setLayout(Layout::Equirectangular);
		}
		else
		{
			throw;
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["pos_x"];
		if (!path.is_null())
		{
			setXposPath(path.get<std::string>());
		}
		else
		{
			setXposPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["neg_x"];
		if (!path.is_null())
		{
			setXnegPath(path.get<std::string>());
		}
		else
		{
			setXnegPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["pos_y"];
		if (!path.is_null())
		{
			setYposPath(path.get<std::string>());
		}
		else
		{
			setYposPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["neg_y"];
		if (!path.is_null())
		{
			setYnegPath(path.get<std::string>());
		}
		else
		{
			setYnegPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["pos_z"];
		if (!path.is_null())
		{
			setZposPath(path.get<std::string>());
		}
		else
		{
			setZposPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["neg_z"];
		if (!path.is_null())
		{
			setZnegPath(path.get<std::string>());
		}
		else
		{
			setZnegPath(std::nullopt);
		}
	}
	
	{
		const nlohmann::ordered_json& path = jsonRoot["equirectangularPath"];
		if (!path.is_null())
		{
			setEquirectangularPath(path.get<std::string>());
		}
		else
		{
			setEquirectangularPath(std::nullopt);
		}
	}
}

void SkyboxAsset::save() const
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 3;
	
	switch (_layout)
	{
		case Layout::Cubemap:
			jsonRoot["layout"] = "cubemap";
			break;
		case Layout::Equirectangular:
			jsonRoot["layout"] = "equirectangular";
			break;
		default:
			throw;
	}

	{
		const std::string* path = getXposPath();
		if (path != nullptr)
		{
			jsonRoot["pos_x"] = *path;
		}
		else
		{
			jsonRoot["pos_x"] = nullptr;
		}
	}

	{
		const std::string* path = getXnegPath();
		if (path != nullptr)
		{
			jsonRoot["neg_x"] = *path;
		}
		else
		{
			jsonRoot["neg_x"] = nullptr;
		}
	}

	{
		const std::string* path = getYposPath();
		if (path != nullptr)
		{
			jsonRoot["pos_y"] = *path;
		}
		else
		{
			jsonRoot["pos_y"] = nullptr;
		}
	}

	{
		const std::string* path = getYnegPath();
		if (path != nullptr)
		{
			jsonRoot["neg_y"] = *path;
		}
		else
		{
			jsonRoot["neg_y"] = nullptr;
		}
	}

	{
		const std::string* path = getZposPath();
		if (path != nullptr)
		{
			jsonRoot["pos_z"] = *path;
		}
		else
		{
			jsonRoot["pos_z"] = nullptr;
		}
	}

	{
		const std::string* path = getZnegPath();
		if (path != nullptr)
		{
			jsonRoot["neg_z"] = *path;
		}
		else
		{
			jsonRoot["neg_z"] = nullptr;
		}
	}
	
	{
		const std::string* path = getEquirectangularPath();
		if (path != nullptr)
		{
			jsonRoot["equirectangularPath"] = *path;
		}
		else
		{
			jsonRoot["equirectangularPath"] = nullptr;
		}
	}
	
	JsonHelper::saveJsonToFile(jsonRoot, FileHelper::getAssetDirectoryPath() / _signature.path);
}

void SkyboxAsset::reload()
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getAssetDirectoryPath() / _signature.path);

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
		case 3:
			deserializeFromVersion3(jsonRoot);
			break;
		default:
			throw;
	}
}

void SkyboxAsset::onChanged()
{
	if (_layout == Layout::Cubemap && _xposPath && _xnegPath && _yposPath && _ynegPath && _zposPath && _znegPath)
	{
		_cubemap = _manager.loadCubemap(
			_xposPath.value(),
			_xnegPath.value(),
			_yposPath.value(),
			_ynegPath.value(),
			_znegPath.value(),
			_zposPath.value(),
			ImageType::Skybox
		);
		_cubemapChangedConnection = _cubemap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else if (_layout == Layout::Equirectangular && _equirectangularPath)
	{
		_cubemap = _manager.loadCubemap(_equirectangularPath.value());
		_cubemapChangedConnection = _cubemap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_cubemap = nullptr;
		_cubemapChangedConnection = {};
	}
}