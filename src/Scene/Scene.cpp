#include <stdexcept>
#include "../Helper/VectorHelper.h"
#include "SceneObject.h"
#include "../Helper/JsonHelper.h"
#include <fmt/core.h>
#include "../Logger.h"
#include "../Engine.h"
#include "Scene.h"
#include "MeshObject.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "../UI/Window/UIInspector.h"

Scene::Scene(Camera camera, std::string name):
_camera(camera), _root(Transform::createSceneRoot()), _name(std::move(name)), _resourceManager(std::thread::hardware_concurrency() - 1)
{

}

Transform* Scene::getRoot()
{
	return _root.get();
}

void Scene::update(double deltaTime)
{
	_camera.update(deltaTime);
	for (std::unique_ptr<SceneObject>& object : _objects)
	{
		object->update(deltaTime);
	}
}

void Scene::add(std::unique_ptr<SceneObject> object)
{
	_objects.emplace_back(std::move(object));
}

void Scene::remove(SceneObject* object)
{
	for (auto& child : object->getTransform().getChildren())
	{
		remove(child->getOwner());
	}
	
	int removeIndex = -1;
	
	for (int i = 0; i < _objects.size(); i++)
	{
		if (_objects[i].get() == object)
			removeIndex = i;
	}
	
	if (removeIndex == -1) throw std::runtime_error("Object passed to Scene.remove is not part of that scene");
	
	VectorHelper::removeAt(_objects, removeIndex);
}

std::vector<std::unique_ptr<SceneObject>>& Scene::getObjects()
{
	return _objects;
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
	nlohmann::json jsonRoot = JsonHelper::loadJsonFromFile(fmt::format("resources/scenes/{}.json", name));
	
	int version = jsonRoot["version"];
	
	if (version < 6)
	{
		Logger::Error("Scene version < 6 are not supported");
		return;
	}
	
	UIInspector::setSelected(std::any());
	
	std::vector<float> cameraPosArray = jsonRoot["camera"]["position"];
	std::vector<float> cameraSphCoordsArray = jsonRoot["camera"]["spherical_coords"];
	
	Camera camera(glm::vec3(cameraPosArray[0], cameraPosArray[1], cameraPosArray[2]),
			glm::vec2(cameraSphCoordsArray[0], cameraSphCoordsArray[1]));
	
	camera.exposure = jsonRoot["camera"]["exposure"];
	
	Engine::setScene(std::make_unique<Scene>(camera, name));
	Scene& scene = Engine::getScene();
	
	if (version < 7 && !jsonRoot["skybox"].empty() || version >= 7 && jsonRoot.contains("skybox"))
	{
		scene.setSkybox(scene.getRM().requestSkybox(jsonRoot["skybox"]["name"]));
		scene.getSkybox()->setRotation(jsonRoot["skybox"]["rotation"]);
	}
	
	for (nlohmann::json& value : jsonRoot["objects"])
	{
		parseSceneObject(value, scene.getRoot(), version, scene);
	}
}

void Scene::parseSceneObject(nlohmann::json& jsonObject, Transform* parent, int version, Scene& scene)
{
	nlohmann::json& jsonData = jsonObject["data"];
	
	std::unique_ptr<SceneObject> sceneObject;
	
	std::vector<float> positionArray = jsonObject["position"];
	glm::vec3 position(positionArray[0], positionArray[1], positionArray[2]);
	
	glm::quat rotation;
	std::vector<float> rotationArray = jsonObject["rotation"];
	if (version >= 7)
	{
		rotation = glm::quat(rotationArray[0], rotationArray[1], rotationArray[2], rotationArray[3]);
	}
	else
	{
		rotation = glm::quat(glm::radians(glm::vec3(rotationArray[0], rotationArray[1], rotationArray[2])));
	}
	
	std::vector<float> scaleArray = jsonObject["scale"];
	glm::vec3 scale(scaleArray[0], scaleArray[1], scaleArray[2]);
	
	std::string name = jsonObject["name"];
	
	std::string type = jsonObject["type"];
	
	if (type == "mesh_object")
	{
		std::vector<float> velocityArray = jsonData["velocity"];
		glm::vec3 velocity(velocityArray[0], velocityArray[1], velocityArray[2]);
		
		std::vector<float> angularVelocityArray = jsonData["angular_velocity"];
		glm::vec3 angularVelocity(angularVelocityArray[0], angularVelocityArray[1], angularVelocityArray[2]);
		
		Material* material = nullptr;
		if (version < 7 && !jsonData["material"].empty() || version >= 7 && jsonData.contains("material"))
		{
			material = scene.getRM().requestMaterial(jsonData["material"]);
		}
		
		Model* model = nullptr;
		if (version < 7 && !jsonData["mesh"].empty() || version >= 7 && jsonData.contains("model"))
		{
			model = scene.getRM().requestModel(jsonData[version >= 7 ? "model" : "mesh"]);
		}
		
		sceneObject = std::make_unique<MeshObject>(parent, material, model, name, position, rotation, scale, velocity, angularVelocity);
		
		static_cast<MeshObject*>(sceneObject.get())->setContributeShadows(jsonData[version >= 7 ? "contribute_shadows" : "contributeShadows"]);
	}
	else if (type == "point_light")
	{
		std::vector<float> colorArray = jsonData["color"];
		glm::vec3 srgbColor(colorArray[0], colorArray[1], colorArray[2]);
		
		float intensity = jsonData["intensity"];
		
		bool castShadows = jsonData[version >= 7 ? "cast_shadows" : "castShadows"];
		
		sceneObject = std::make_unique<PointLight>(parent, name, position, rotation, scale, srgbColor, intensity, castShadows);
	}
	else if (type == "directional_light")
	{
		std::vector<float> colorArray = jsonData["color"];
		glm::vec3 srgbColor(colorArray[0], colorArray[1], colorArray[2]);
		
		float intensity = jsonData["intensity"];
		
		bool castShadows = jsonData[version >= 7 ? "cast_shadows" : "castShadows"];
		
		sceneObject = std::make_unique<DirectionalLight>(parent, name, position, rotation, scale, srgbColor, intensity, castShadows);
	}
	else
	{
		throw std::runtime_error(fmt::format("The object type {} is not recognized", jsonObject["type"]));
	}
	
	SceneObject* sceneObjectPtr = sceneObject.get();
	
	scene.add(std::move(sceneObject));
	
	if (version >= 7 && jsonObject.contains("children") || version < 7)
	{
		for (nlohmann::json& value : jsonObject["children"])
		{
			parseSceneObject(value, &sceneObjectPtr->getTransform(), version, scene);
		}
	}
}

void Scene::save()
{
	nlohmann::json jsonRoot;
	
	jsonRoot["version"] = 7;
	
	
	nlohmann::json jsonCamera;
	glm::vec3 cameraPosition = _camera.position;
	jsonCamera["position"] = {cameraPosition.x, cameraPosition.y, cameraPosition.z};
	glm::vec2 cameraRotation = _camera.sphericalCoords;
	jsonCamera["spherical_coords"] = {cameraRotation.x, cameraRotation.y};
	jsonCamera["exposure"] = _camera.exposure;
	
	jsonRoot["camera"] = jsonCamera;
	
	
	if (_skybox != nullptr)
	{
		nlohmann::json jsonSkybox;
		jsonSkybox["name"] = _skybox->getName();
		jsonSkybox["rotation"] = _skybox->getRotation();
		jsonRoot["skybox"] = jsonSkybox;
	}
	
	
	nlohmann::json objects;
	
	for (Transform* transform : _root->getChildren())
	{
		objects.push_back(serializeSceneObject(transform));
	}
	
	jsonRoot["objects"] = objects;
	
	JsonHelper::saveJsonToFile(jsonRoot, fmt::format("resources/scenes/{}.json", _name));
}

nlohmann::json Scene::serializeSceneObject(Transform* transform)
{
	nlohmann::json jsonObject;
	
	glm::vec3 position = transform->getPosition();
	jsonObject["position"] = {position.x, position.y, position.z};
	glm::quat rotation = transform->getRotation();
	jsonObject["rotation"] = {rotation.w, rotation.x, rotation.y, rotation.z};
	glm::vec3 scale = transform->getScale();
	jsonObject["scale"] = {scale.x, scale.y, scale.z};
	
	jsonObject["name"] = transform->getOwner()->getName();
	
	
	nlohmann::json jsonData;
	
	MeshObject* meshObject = dynamic_cast<MeshObject*>(transform->getOwner());
	if (meshObject != nullptr)
	{
		jsonObject["type"] = "mesh_object";
		
		glm::vec3 velocity = meshObject->getVelocity();
		jsonData["velocity"] = {velocity.x, velocity.y, velocity.z};
		glm::vec3 angularVelocity = meshObject->getAngularVelocity();
		jsonData["angular_velocity"] = {angularVelocity.x, angularVelocity.y, angularVelocity.z};
		
		Model* model = meshObject->getModel();
		if (model != nullptr)
		{
			jsonData["model"] = model->getName();
		}
		
		Material* material = meshObject->getMaterial();
		if (material != nullptr)
		{
			jsonData["material"] = material->getName();
		}
		
		jsonData["contribute_shadows"] = meshObject->getContributeShadows();
	}
	
	Light* light = dynamic_cast<Light*>(transform->getOwner());
	if (light != nullptr)
	{
		glm::vec3 color = light->getSrgbColor();
		jsonData["color"] = {color.r, color.g, color.b};
		jsonData["intensity"] = light->getIntensity();
	}
	
	PointLight* pointLight = dynamic_cast<PointLight*>(transform->getOwner());
	if (pointLight != nullptr)
	{
		jsonObject["type"] = "point_light";
		
		jsonData["cast_shadows"] = pointLight->getCastShadows();
	}
	
	DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(transform->getOwner());
	if (directionalLight != nullptr)
	{
		jsonObject["type"] = "directional_light";
		
		jsonData["cast_shadows"] = directionalLight->getCastShadows();
	}
	
	jsonObject["data"] = jsonData;
	
	
	if (!transform->getChildren().empty())
	{
		nlohmann::json jsonChildren;
		
		for (Transform* childTransform : transform->getChildren())
		{
			jsonChildren.push_back(serializeSceneObject(childTransform));
		}
		
		jsonObject["children"] = jsonChildren;
	}
	
	return jsonObject;
}

Scene::~Scene()
{
	for (auto& object : _objects)
	{
		object.release();
	}
	
	_root.release();
}

const std::string& Scene::getName() const
{
	return _name;
}
