#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>
#include <sigslot/signal.hpp>

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

	void setSkybox(std::optional<std::string_view> path);
	SkyboxAsset* getSkybox();
	
	float getSkyboxRotation() const;
	void setSkyboxRotation(float rotation);
	
	static void load(const std::filesystem::path& path);
	void save(const std::filesystem::path& path);
	
	const std::string& getName() const;
	void setName(const std::string& name);
	
	static uint64_t getChangeVersion();

private:
	struct EntityContainer
	{
		std::unique_ptr<Entity> entity;
		sigslot::scoped_connection entityChangedConnection;
	};
	
	std::unique_ptr<Transform> _root;
	std::vector<EntityContainer> _entities;
	std::string _name = "Untitled Scene";
	
	SkyboxAsset* _skybox = nullptr;
	sigslot::scoped_connection _skyboxChangedConnection;
	float _skyboxRotation = 0;
	
	static std::atomic_uint64_t _changeVersion;
	
	static void deserializeEntity(const nlohmann::ordered_json& json, Transform& parent, int version, Scene& scene);
	nlohmann::ordered_json serializeEntity(const Entity& entity) const;
	
	friend class EntityIterator;
	friend class EntityConstIterator;
};