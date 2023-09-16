#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

class MaterialAsset;
class MeshAsset;

class ModelRenderer : public Component
{
public:
	struct RenderData
	{
		Transform&     transform;
		MaterialAsset& material;
		MeshAsset&     mesh;
		bool           contributeShadows;
		Entity&        owner;
	};
	
	explicit ModelRenderer(Entity& entity);

	void setMaterial(std::optional<std::string_view> path);
	MaterialAsset* getMaterial() const;
	
	void setMesh(std::optional<std::string_view> path);
	MeshAsset* getMesh() const;
	
	bool getContributeShadows() const;
	void setContributeShadows(bool contributeShadows);
	
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera) override;
	void onDrawUi() override;
	
	void duplicate(Entity& targetEntity) const override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	MaterialAsset* _material = nullptr;
	sigslot::scoped_connection _materialChangedConnection;
	
	MeshAsset* _mesh = nullptr;
	sigslot::scoped_connection _meshChangedConnection;
	
	bool _contributeShadows = true;
	
	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion3(const nlohmann::ordered_json& jsonRoot);
};