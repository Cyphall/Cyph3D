#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>

class SkyboxAsset;
class Transform;
class Entity;
class EntityIterator;
class EntityConstIterator;
class SceneRenderer;
class Camera;

class Scene
{
public:
	explicit Scene(std::string name = "Untitled Scene");
	~Scene();
	
	void onUpdate();
	void onPreRender(SceneRenderer& sceneRenderer, Camera& camera);
	
	Entity& createEntity(Transform& parent);
	EntityIterator findEntity(Entity& entity);
	EntityIterator removeEntity(EntityIterator where);
	
	EntityIterator entities_begin();
	EntityIterator entities_end();
	EntityConstIterator entities_cbegin() const;
	EntityConstIterator entities_cend() const;
	
	Transform& getRoot();

	const std::string* getSkyboxPath() const;
	void setSkyboxPath(std::optional<std::string_view> path);
	SkyboxAsset* getSkybox();
	
	float getSkyboxRotation() const;
	void setSkyboxRotation(float rotation);
	
	static void load(const std::filesystem::path& path);
	void save(const std::filesystem::path& path);
	
	const std::string& getName() const;

private:
	std::unique_ptr<Transform> _root;
	std::list<std::unique_ptr<Entity>> _entities;
	std::string _name;
	
	std::optional<std::string> _skyboxPath;
	SkyboxAsset* _skybox = nullptr;
	float _skyboxRotation = 0;
	
	static void deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene);
	nlohmann::ordered_json serializeEntity(const Entity& entity) const;
};