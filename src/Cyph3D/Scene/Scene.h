#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>

class SkyboxAsset;
class Transform;
class Entity;
class EntityIterator;
class EntityConstIterator;
class RenderRegistry;
class Camera;

class Scene
{
public:
	Scene();
	~Scene();
	
	void onUpdate();
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera);
	
	Entity& createEntity(Transform& parent);
	EntityIterator findEntity(Entity& entity);
	EntityIterator removeEntity(EntityIterator where);
	
	EntityIterator begin();
	EntityIterator end();
	EntityConstIterator begin() const;
	EntityConstIterator end() const;
	
	Transform& getRoot();

	const std::string* getSkyboxPath() const;
	void setSkyboxPath(std::optional<std::string_view> path);
	SkyboxAsset* getSkybox();
	
	float getSkyboxRotation() const;
	void setSkyboxRotation(float rotation);
	
	static void load(const std::filesystem::path& path);
	void save(const std::filesystem::path& path);
	
	const std::string& getName() const;
	void setName(const std::string& name);

private:
	std::unique_ptr<Transform> _root;
	std::vector<std::unique_ptr<Entity>> _entities;
	std::string _name = "Untitled Scene";
	
	std::optional<std::string> _skyboxPath;
	SkyboxAsset* _skybox = nullptr;
	float _skyboxRotation = 0;
	
	static void deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene);
	nlohmann::ordered_json serializeEntity(const Entity& entity) const;
};