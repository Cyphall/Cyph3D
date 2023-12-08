#include "Scene.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/Component.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Iterator/EntityConstIterator.h"
#include "Cyph3D/Iterator/EntityIterator.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/UI/Window/UIViewport.h"

#include <format>
#include <glm/gtc/type_ptr.hpp>

std::atomic_uint64_t Scene::_changeVersion = 0;

Scene::Scene():
	_root(Transform::createSceneRoot())
{
	_changeVersion++;
}

Scene::~Scene()
{
	_entities.clear();

	_root.reset();
}

void Scene::onUpdate()
{
	for (Entity& entity : *this)
	{
		entity.onUpdate();
	}
}

void Scene::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{
	for (Entity& entity : *this)
	{
		entity.onPreRender(renderRegistry, camera);
	}
}

Entity& Scene::createEntity(Transform& parent)
{
	EntityContainer& container = _entities.emplace_back();
	container.entity = std::make_unique<Entity>(parent, *this);
	container.entityChangedConnection = container.entity->getChangedSignal().connect(
		[]()
		{
			_changeVersion++;
		}
	);

	_changeVersion++;

	return *container.entity;
}

EntityIterator Scene::findEntity(const Entity& entity)
{
	return std::find_if(
		begin(), end(),
		[&](const Entity& e)
		{
			return &e == &entity;
		}
	);
}

EntityIterator Scene::removeEntity(EntityIterator where)
{
	std::vector<Transform*> children = where->getTransform().getChildren();
	for (Transform* child : children)
	{
		auto it = findEntity(*child->getOwner());
		if (it == end())
		{
			throw;
		}
		removeEntity(it);
	}

	EntityIterator newIt = EntityIterator(_entities.erase(where.getUnderlyingIterator()));

	_changeVersion++;

	return newIt;
}

EntityIterator Scene::begin()
{
	return EntityIterator(_entities.begin());
}

EntityIterator Scene::end()
{
	return EntityIterator(_entities.end());
}

EntityConstIterator Scene::begin() const
{
	return EntityConstIterator(_entities.cbegin());
}

EntityConstIterator Scene::end() const
{
	return EntityConstIterator(_entities.cend());
}

Transform& Scene::getRoot()
{
	return *_root;
}

void Scene::setSkybox(std::optional<std::string_view> path)
{
	if (path)
	{
		_skybox = Engine::getAssetManager().loadSkybox(*path);
		_skyboxChangedConnection = _skybox->getChangedSignal().connect(
			[]()
			{
				_changeVersion++;
			}
		);
	}
	else
	{
		_skybox = nullptr;
		_skyboxChangedConnection = {};
	}

	_changeVersion++;
}

SkyboxAsset* Scene::getSkybox()
{
	return _skybox;
}

float Scene::getSkyboxRotation() const
{
	return _skyboxRotation;
}

void Scene::setSkyboxRotation(float rotation)
{
	_skyboxRotation = rotation;

	_changeVersion++;
}

void Scene::load(const std::filesystem::path& path)
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getAssetDirectoryPath() / path);

	int version = jsonRoot["version"].get<int>();

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();

	scene->setName(path.filename().replace_extension().generic_string());

	Camera camera;
	const nlohmann::ordered_json& jsonCamera = jsonRoot["camera"];

	const nlohmann::ordered_json& jsonCameraPosition = jsonCamera["position"];
	glm::vec3 cameraPosition = {
		jsonCameraPosition.at(0).get<float>(),
		jsonCameraPosition.at(1).get<float>(),
		jsonCameraPosition.at(2).get<float>()
	};
	camera.setPosition(cameraPosition);

	const nlohmann::ordered_json& jsonCameraSphericalCoords = jsonCamera["spherical_coords"];
	glm::vec2 cameraSphericalCoords = {
		jsonCameraSphericalCoords.at(0).get<float>(),
		jsonCameraSphericalCoords.at(1).get<float>()
	};
	if (version <= 2)
	{
		cameraSphericalCoords.x = 180.0f - cameraSphericalCoords.x;
	}
	camera.setSphericalCoords(cameraSphericalCoords);

	if (version <= 4)
	{
		float exposure = 1.0f / std::pow(2.0f, jsonCamera["exposure"].get<float>());
		camera.setAperture(1.0f);
		camera.setShutterSpeed({1.2f, 1.0f});
		camera.setSensitivity(100.0f * (1.0f / exposure));
	}
	else
	{
		camera.setAperture(jsonCamera["aperture"].get<float>());
		const nlohmann::ordered_json& jsonCameraShutterSpeed = jsonCamera["shutterSpeed"];
		camera.setShutterSpeed({jsonCameraShutterSpeed.at(0).get<float>(), jsonCameraShutterSpeed.at(1).get<float>()});
		camera.setSensitivity(jsonCamera["sensitivity"].get<int>());
	}

	UIViewport::setCamera(camera);

	if (version <= 1)
	{
		nlohmann::ordered_json& jsonSkybox = jsonRoot["skybox"];
		if (!jsonSkybox.is_null())
		{
			Logger::info("Scene deseralization: converting skybox identifier from version 1.");
			std::string oldName = jsonSkybox["name"].get<std::string>();
			std::string newFileName = std::filesystem::path(oldName).filename().generic_string();
			std::string convertedPath = std::format("skyboxes/{}/{}.c3dskybox", oldName, newFileName);
			if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
			{
				scene->setSkybox(convertedPath);
			}
			else
			{
				Logger::warning("Scene deseralization: unable to convert skybox identifier from version 1.");
			}

			scene->setSkyboxRotation(jsonSkybox["rotation"].get<float>());
		}
	}
	else if (version <= 3)
	{
		nlohmann::ordered_json& jsonSkybox = jsonRoot["skybox"];
		if (!jsonSkybox.is_null())
		{
			scene->setSkybox(jsonSkybox["name"].get<std::string>());

			scene->setSkyboxRotation(jsonSkybox["rotation"].get<float>());
		}
	}
	else
	{
		nlohmann::ordered_json& jsonSkyboxPath = jsonRoot["skybox"];
		if (!jsonSkyboxPath.is_null())
		{
			scene->setSkybox(jsonSkyboxPath.get<std::string>());
		}

		scene->setSkyboxRotation(jsonRoot["skybox_rotation"].get<float>());
	}

	for (nlohmann::ordered_json& value : jsonRoot["entities"])
	{
		deserializeEntity(value, scene->getRoot(), version, *scene);
	}

	Engine::setScene(std::move(scene));
}

void Scene::deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene)
{
	ObjectSerialization serialization = ObjectSerialization::fromJson(json);

	Entity& entity = scene.createEntity(parent);
	entity.deserialize(serialization);

	for (const nlohmann::ordered_json& child : json["children"])
	{
		deserializeEntity(child, entity.getTransform(), version, scene);
	}
}

void Scene::save(const std::filesystem::path& path)
{
	_name = path.filename().replace_extension().generic_string();

	nlohmann::ordered_json jsonRoot;

	jsonRoot["version"] = 5;

	const Camera& camera = UIViewport::getCamera();
	nlohmann::ordered_json jsonCamera;
	glm::vec3 cameraPosition = camera.getPosition();
	jsonCamera["position"] = {cameraPosition.x, cameraPosition.y, cameraPosition.z};
	glm::vec2 cameraRotation = camera.getSphericalCoords();
	jsonCamera["spherical_coords"] = {cameraRotation.x, cameraRotation.y};

	jsonCamera["aperture"] = camera.getAperture();
	jsonCamera["shutterSpeed"] = {camera.getShutterSpeed().x, camera.getShutterSpeed().y};
	jsonCamera["sensitivity"] = camera.getSensitivity();

	jsonRoot["camera"] = jsonCamera;

	if (_skybox)
	{
		jsonRoot["skybox"] = _skybox->getSignature().path;
	}
	else
	{
		jsonRoot["skybox"] = nullptr;
	}

	jsonRoot["skybox_rotation"] = getSkyboxRotation();

	std::vector<nlohmann::ordered_json> entities;
	entities.reserve(_root->getChildren().size());
	for (Transform* child : _root->getChildren())
	{
		entities.push_back(serializeEntity(*child->getOwner()));
	}

	jsonRoot["entities"] = entities;

	JsonHelper::saveJsonToFile(jsonRoot, path.generic_string());
}

nlohmann::ordered_json Scene::serializeEntity(const Entity& entity)
{
	nlohmann::ordered_json jsonData = entity.serialize().toJson();

	const Transform& transform = entity.getTransform();
	std::vector<nlohmann::ordered_json> children;
	children.reserve(transform.getChildren().size());
	for (Transform* child : transform.getChildren())
	{
		children.push_back(serializeEntity(*child->getOwner()));
	}
	jsonData["children"] = children;

	return jsonData;
}

const std::string& Scene::getName() const
{
	return _name;
}

void Scene::setName(const std::string& name)
{
	_name = name;

	_changeVersion++;
}

uint64_t Scene::getChangeVersion()
{
	return _changeVersion;
}
