#include <stdexcept>
#include "Cyph3D/Helper/VectorHelper.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Helper/ThreadHelper.h"
#include "Cyph3D/Iterator/EntityIterator.h"
#include "Cyph3D/Iterator/EntityConstIterator.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/Entity/Component/Component.h"

Scene::Scene(std::string name):
_root(Transform::createSceneRoot()), _name(std::move(name)), _resourceManager(std::max(ThreadHelper::getPhysicalCoreCount() - 2, 1))
{

}

Transform& Scene::getRoot()
{
	return *_root.get();
}

void Scene::onUpdate()
{
	for (auto it = entities_begin(); it != entities_end(); it++)
	{
		(*it).onUpdate();
	}
}

void Scene::onPreRender(RenderContext& context)
{
	for (auto it = entities_begin(); it != entities_end(); it++)
	{
		(*it).onPreRender(context);
	}
}

Entity& Scene::createEntity(Transform& parent)
{
	return *_entities.emplace_back(new Entity(parent, *this)).get();
}

EntityIterator Scene::removeEntity(EntityIterator where)
{
	std::vector<Transform*> children = where->getTransform().getChildren();
	for (Transform* child : children)
	{
		for (auto it = _entities.begin(); it != _entities.end(); it++)
		{
			if (it->get() == child->getOwner())
			{
				removeEntity(EntityIterator(it));
				break;
			}
		}
	}
	
	return EntityIterator(_entities.erase(where.getUnderlyingIterator()));
}

Skybox* Scene::getSkybox()
{
	return _skybox;
}

void Scene::setSkybox(Skybox* skybox)
{
	_skybox = skybox;
}

ResourceManager& Scene::getRM()
{
	return _resourceManager;
}

void Scene::load(const std::filesystem::path& path)
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(path.generic_string());

	int version = jsonRoot["version"].get<int>();
	
	Engine::setScene(std::make_unique<Scene>(path.filename().replace_extension().generic_string()));
	Scene& scene = Engine::getScene();
	
	Camera camera(glm::make_vec3(jsonRoot["camera"]["position"].get<std::vector<float>>().data()),
		glm::make_vec2(jsonRoot["camera"]["spherical_coords"].get<std::vector<float>>().data()));

	camera.setExposure(jsonRoot["camera"]["exposure"].get<float>());
	
	UIViewport::setCamera(camera);

	if (!jsonRoot["skybox"].is_null())
	{
		scene.setSkybox(scene.getRM().requestSkybox(jsonRoot["skybox"]["name"].get<std::string>()));
		scene.getSkybox()->setRotation(jsonRoot["skybox"]["rotation"].get<float>());
	}

	for (nlohmann::ordered_json& value : jsonRoot["entities"])
	{
		deserializeEntity(value, scene.getRoot(), version, scene);
	}
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
	
	jsonRoot["version"] = 1;
	
	const Camera& camera = UIViewport::getCamera();
	nlohmann::ordered_json jsonCamera;
	glm::vec3 cameraPosition = camera.getPosition();
	jsonCamera["position"] = {cameraPosition.x, cameraPosition.y, cameraPosition.z};
	glm::vec2 cameraRotation = camera.getSphericalCoords();
	jsonCamera["spherical_coords"] = {cameraRotation.x, cameraRotation.y};
	jsonCamera["exposure"] = camera.getExposure();
	
	jsonRoot["camera"] = jsonCamera;
	
	nlohmann::ordered_json jsonSkybox;
	if (_skybox != nullptr)
	{
		jsonSkybox["name"] = _skybox->getName();
		jsonSkybox["rotation"] = _skybox->getRotation();
	}
	jsonRoot["skybox"] = jsonSkybox;
	
	std::vector<nlohmann::ordered_json> entities;
	entities.reserve(_root->getChildren().size());
	for (Transform* child : _root->getChildren())
	{
		entities.push_back(serializeEntity(*child->getOwner()));
	}
	
	jsonRoot["entities"] = entities;
	
	JsonHelper::saveJsonToFile(jsonRoot, path.generic_string());
}

nlohmann::ordered_json Scene::serializeEntity(const Entity& entity) const
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

Scene::~Scene()
{
	_entities.clear();
	
	_root.reset();
}

const std::string& Scene::getName() const
{
	return _name;
}

EntityIterator Scene::entities_begin()
{
	return EntityIterator(_entities.begin());
}

EntityIterator Scene::entities_end()
{
	return EntityIterator(_entities.end());
}

EntityConstIterator Scene::entities_cbegin() const
{
	return EntityConstIterator(_entities.cbegin());
}

EntityConstIterator Scene::entities_cend() const
{
	return EntityConstIterator(_entities.cend());
}