#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <memory>

class MaterialAsset;
class MeshAsset;

class ModelRenderer : public Component
{
public:
	struct RenderData
	{
		MaterialAsset* material;
		MeshAsset* mesh;
		glm::mat4 matrix;
		bool contributeShadows;
		Entity* owner;
	};
	
	explicit ModelRenderer(Entity& entity);

	const std::string* getMaterialPath() const;
	void setMaterialPath(std::optional<std::string_view> path);
	MaterialAsset* getMaterial() const;
	
	const std::string* getMeshPath() const;
	void setMeshPath(std::optional<std::string_view> path);
	MeshAsset* getMesh() const;
	
	bool getContributeShadows() const;
	void setContributeShadows(bool contributeShadows);
	
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera) override;
	void onDrawUi() override;
	
	void duplicate(Entity& targetEntity) const override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& modelRendererSerialization) override;

private:
	MaterialAsset* _material = nullptr;
	sigslot::scoped_connection _materialChangedConnection;
	
	MeshAsset* _mesh = nullptr;
	sigslot::scoped_connection _meshChangedConnection;
	
	bool _contributeShadows = true;
};