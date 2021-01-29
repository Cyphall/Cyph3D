#pragma once

#include "SceneObject.h"
#include "../ResourceManagement/Skybox.h"
#include "../ResourceManagement/ResourceManager.h"
#include "Camera.h"
#include <nlohmann/json.hpp>

class Scene
{
public:
	Scene(Camera camera = Camera(), std::string name = "Untitled Scene");
	~Scene();
	Transform* getRoot();
	void update();
	
	void add(std::unique_ptr<SceneObject> object);
	void remove(SceneObject* object);
	
	std::vector<std::unique_ptr<SceneObject>>& getObjects();
	Camera& getCamera();
	
	Skybox* getSkybox();
	void setSkybox(Skybox* skybox);
	
	ResourceManager& getRM();
	
	static void load(const std::string& name);
	void save();
	
	const std::string& getName() const;

private:
	std::unique_ptr<Transform> _root;
	std::vector<std::unique_ptr<SceneObject>> _objects;
	Camera _camera;
	std::string _name;
	Skybox* _skybox = nullptr;
	ResourceManager _resourceManager;
	
	static void parseSceneObject(nlohmann::json& jsonObject, Transform* parent, int version, Scene& scene);
	nlohmann::json serializeSceneObject(Transform* transform);
};
