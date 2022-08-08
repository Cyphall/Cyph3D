#pragma once

#include "Cyph3D/ResourceManagement/ResourceManager.h"

#include <nlohmann/json.hpp>
#include <filesystem>

class Skybox;
class Transform;
class Entity;
class EntityIterator;
class EntityConstIterator;
struct RenderContext;

class Scene
{
public:
	explicit Scene(std::string name = "Untitled Scene");
	~Scene();
	
	void onUpdate();
	void onPreRender(RenderContext& context);
	
	Entity& createEntity(Transform& parent);
	EntityIterator removeEntity(EntityIterator where);
	
	EntityIterator entities_begin();
	EntityIterator entities_end();
	EntityConstIterator entities_cbegin() const;
	EntityConstIterator entities_cend() const;
	
	Transform& getRoot();
	
	Skybox* getSkybox();
	void setSkybox(Skybox* skybox);
	
	ResourceManager& getRM();
	
	static void load(const std::filesystem::path& path);
	void save(const std::filesystem::path& path);
	
	const std::string& getName() const;

private:
	std::unique_ptr<Transform> _root;
	std::list<std::unique_ptr<Entity>> _entities;
	std::string _name;
	Skybox* _skybox = nullptr;
	ResourceManager _resourceManager;
	
	static void deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene);
	nlohmann::ordered_json serializeEntity(const Entity& entity) const;
};