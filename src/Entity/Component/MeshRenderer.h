#pragma once

#include "Component.h"
#include "../../GLObject/Material.h"
#include "../../ResourceManagement/Model.h"

class MeshRenderer : public Component
{
public:
	struct RenderData
	{
		const Mesh* mesh;
		Material* material;
		glm::mat4 matrix;
		bool contributeShadows;
		Entity* owner;
	};
	
	MeshRenderer(Entity& entity);
	
	Material* getMaterial() const;
	void setMaterial(Material* material);
	
	Model* getModel() const;
	void setModel(Model* model);
	
	bool getContributeShadows() const;
	void setContributeShadows(bool contributeShadows);
	
	void onPreRender() override;
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ComponentSerialization serialize() const override;
	void deserialize(const ComponentSerialization& data) override;

private:
	Material* _material = nullptr;
	Model* _model = nullptr;
	bool _contributeShadows = true;
	
	Material* getDrawingMaterial();
};
