#pragma once

#include "../ResourceManagement/Skybox.h"
#include "../ResourceManagement/ResourceManager.h"
#include "Camera.h"
#include "Transform.h"
#include "../Iterator/EntityIterator.h"
#include "../Iterator/EntityConstIterator.h"

#include <nlohmann/json.hpp>

class Scene
{
public:
	Scene(Camera camera = Camera(), std::string name = "Untitled Scene");
	~Scene();
	
	void onUpdate();
	void onPreRender();
	
	Entity& createEntity(Transform& parent);
	EntityIterator removeEntity(EntityIterator where);
	
	EntityIterator entities_begin();
	EntityIterator entities_end();
	EntityConstIterator entities_cbegin() const;
	EntityConstIterator entities_cend() const;
	
	Transform& getRoot();
	
	Camera& getCamera();
	
	Skybox* getSkybox();
	void setSkybox(Skybox* skybox);
	
	ResourceManager& getRM();
	
	static void load(const std::string& name);
	static void load_old(const std::string& name);
	void save();
	
	const std::string& getName() const;

private:
	std::unique_ptr<Transform> _root;
	std::list<std::unique_ptr<Entity>> _entities;
	Camera _camera;
	std::string _name;
	Skybox* _skybox = nullptr;
	ResourceManager _resourceManager;
	
	static void deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene);
	static void parseSceneObject_old(nlohmann::json& jsonObject, Transform* parent, int version, Scene& scene);
	nlohmann::ordered_json serializeEntity(const Entity& entity);
};
