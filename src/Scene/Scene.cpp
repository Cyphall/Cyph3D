#include <stdexcept>
#include "../Helper/VectorHelper.h"
#include "../Entity/Entity.h"
#include "../Helper/JsonHelper.h"
#include <fmt/core.h>
#include <glm/gtc/type_ptr.hpp>
#include "../Logger.h"
#include "../Engine.h"
#include "Scene.h"
#include "../UI/Window/UIInspector.h"
#include "../Entity/Component/Animator.h"
#include "../Entity/Component/MeshRenderer.h"
#include "../Entity/Component/PointLight.h"
#include "../Entity/Component/DirectionalLight.h"

Scene::Scene(Camera camera, std::string name):
_camera(camera), _root(Transform::createSceneRoot()), _name(std::move(name)), _resourceManager(std::thread::hardware_concurrency() - 1)
{

}

Transform& Scene::getRoot()
{
	return *_root.get();
}

void Scene::onUpdate()
{
	_camera.update();
	
	for (auto it = entities_begin(); it != entities_end(); it++)
	{
		(*it).onUpdate();
	}
}

void Scene::onPreRender()
{
	for (auto it = entities_begin(); it != entities_end(); it++)
	{
		(*it).onPreRender();
	}
}

Entity& Scene::addEntity(Transform& parent)
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

Camera& Scene::getCamera()
{
	return _camera;
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

void Scene::load(const std::string& name)
{
	UIInspector::setSelected(std::any());
	
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(fmt::format("resources/scenes/{}.json", name));

	int version = jsonRoot["version"].get<int>();
	
	if (version > 1)
	{
		load_old(name);
		return;
	}
	
	Camera camera(glm::make_vec3(jsonRoot["camera"]["position"].get<std::vector<float>>().data()),
		glm::make_vec2(jsonRoot["camera"]["spherical_coords"].get<std::vector<float>>().data()));

	camera.setExposure(jsonRoot["camera"]["exposure"].get<float>());

	Engine::setScene(std::make_unique<Scene>(camera, name));
	Scene& scene = Engine::getScene();

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
	Entity& entity = scene.addEntity(parent);
	EntitySerialization serialization(json["version"].get<int>());
	serialization.json = json["data"];
	entity.deserialize(serialization);
	
	for (const nlohmann::ordered_json& child : json["children"])
	{
		deserializeEntity(child, entity.getTransform(), version, scene);
	}
}

void Scene::save()
{
	nlohmann::ordered_json jsonRoot;
	
	jsonRoot["version"] = 1;
	
	
	nlohmann::ordered_json jsonCamera;
	glm::vec3 cameraPosition = _camera.getPosition();
	jsonCamera["position"] = {cameraPosition.x, cameraPosition.y, cameraPosition.z};
	glm::vec2 cameraRotation = _camera.getSphericalCoords();
	jsonCamera["spherical_coords"] = {cameraRotation.x, cameraRotation.y};
	jsonCamera["exposure"] = _camera.getExposure();
	
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
	
	JsonHelper::saveJsonToFile(jsonRoot, fmt::format("resources/scenes/{}.json", _name));
}

nlohmann::ordered_json Scene::serializeEntity(const Entity& entity)
{
	nlohmann::ordered_json jsonData;
	
	EntitySerialization serialization = entity.serialize();
	jsonData["version"] = serialization.version;
	jsonData["data"] = serialization.json;
	
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
	
	_root.release();
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

void Scene::load_old(const std::string& name)
{
	nlohmann::json jsonRoot = JsonHelper::loadJsonFromFile(fmt::format("resources/scenes/{}.json", name));

	int version = jsonRoot["version"].get<int>();

	UIInspector::setSelected(std::any());

	std::vector<float> cameraPosArray = jsonRoot["camera"]["position"].get<std::vector<float>>();
	std::vector<float> cameraSphCoordsArray = jsonRoot["camera"]["spherical_coords"].get<std::vector<float>>();

	Camera camera(glm::vec3(cameraPosArray[0], cameraPosArray[1], cameraPosArray[2]),
			glm::vec2(cameraSphCoordsArray[0], cameraSphCoordsArray[1]));

	camera.setExposure(jsonRoot["camera"]["exposure"].get<float>());

	Engine::setScene(std::make_unique<Scene>(camera, name));
	Scene& scene = Engine::getScene();

	if (jsonRoot.contains("skybox"))
	{
		scene.setSkybox(scene.getRM().requestSkybox(jsonRoot["skybox"]["name"].get<std::string>()));
		scene.getSkybox()->setRotation(jsonRoot["skybox"]["rotation"].get<float>());
	}

	for (nlohmann::json& value : jsonRoot["objects"])
	{
		parseSceneObject_old(value, &scene.getRoot(), version, scene);
	}
}

void Scene::parseSceneObject_old(nlohmann::json& jsonObject, Transform* parent, int version, Scene& scene)
{
	nlohmann::json& jsonData = jsonObject["data"];
	
	Entity& entity = scene.addEntity(*parent);
	Transform& transform = entity.getTransform();

	std::vector<float> positionArray = jsonObject["position"].get<std::vector<float>>();
	glm::vec3 position(positionArray[0], positionArray[1], positionArray[2]);
	transform.setLocalPosition(position);

	std::vector<float> rotationArray = jsonObject["rotation"].get<std::vector<float>>();
	glm::quat rotation = glm::quat(rotationArray[0], rotationArray[1], rotationArray[2], rotationArray[3]);
	transform.setLocalRotation(rotation);

	std::vector<float> scaleArray = jsonObject["scale"].get<std::vector<float>>();
	glm::vec3 scale(scaleArray[0], scaleArray[1], scaleArray[2]);
	transform.setLocalScale(scale);

	std::string name = jsonObject["name"].get<std::string>();
	entity.setName(name);

	std::string type = jsonObject["type"].get<std::string>();

	if (type == "mesh_object")
	{
		std::vector<float> velocityArray = jsonData["velocity"].get<std::vector<float>>();
		glm::vec3 velocity(velocityArray[0], velocityArray[1], velocityArray[2]);

		std::vector<float> angularVelocityArray = jsonData["angular_velocity"].get<std::vector<float>>();
		glm::vec3 angularVelocity(angularVelocityArray[0], angularVelocityArray[1], angularVelocityArray[2]);
		
		if (velocity != glm::vec3(0) || angularVelocity != glm::vec3(0))
		{
			Animator& animator = entity.addComponent<Animator>();
			animator.setVelocity(velocity);
			animator.setAngularVelocity(angularVelocity);
		}

		Material* material = nullptr;
		if (jsonData.contains("material"))
		{
			material = scene.getRM().requestMaterial(jsonData["material"].get<std::string>());
		}

		Model* model = nullptr;
		if (jsonData.contains("model"))
		{
			model = scene.getRM().requestModel(jsonData["model"].get<std::string>());
		}
		
		bool contributeShadows = jsonData["contribute_shadows"].get<bool>();
		
		MeshRenderer& meshRenderer = entity.addComponent<MeshRenderer>();
		meshRenderer.setMaterial(material);
		meshRenderer.setModel(model);
		meshRenderer.setContributeShadows(contributeShadows);
	}
	else if (type == "point_light")
	{
		PointLight& pointLight = entity.addComponent<PointLight>();
		
		std::vector<float> colorArray = jsonData["color"].get<std::vector<float>>();
		glm::vec3 srgbColor(colorArray[0], colorArray[1], colorArray[2]);
		pointLight.setSrgbColor(srgbColor);

		float intensity = jsonData["intensity"].get<float>();
		pointLight.setIntensity(intensity);

		bool castShadows = jsonData["cast_shadows"].get<bool>();
		pointLight.setCastShadows(castShadows);
		
		if (version >= 8)
		{
			int shadowResolution = jsonData["shadow_resolution"].get<int>();
			pointLight.setResolution(shadowResolution);
		}
	}
	else if (type == "directional_light")
	{
		DirectionalLight& directionalLight = entity.addComponent<DirectionalLight>();
		
		std::vector<float> colorArray = jsonData["color"].get<std::vector<float>>();
		glm::vec3 srgbColor(colorArray[0], colorArray[1], colorArray[2]);
		directionalLight.setSrgbColor(srgbColor);
		
		float intensity = jsonData["intensity"].get<float>();
		directionalLight.setIntensity(intensity);
		
		bool castShadows = jsonData["cast_shadows"].get<bool>();
		directionalLight.setCastShadows(castShadows);
		
		if (version >= 8)
		{
			int shadowResolution = jsonData["shadow_resolution"].get<int>();
			directionalLight.setResolution(shadowResolution);
		}
	}
	else
	{
		throw std::runtime_error(fmt::format("The object type {} is not recognized", jsonObject["type"]));
	}

	if (jsonObject.contains("children"))
	{
		for (nlohmann::json& value : jsonObject["children"])
		{
			parseSceneObject_old(value, &transform, version, scene);
		}
	}
}
