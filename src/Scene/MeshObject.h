#pragma once

#include "../ResourceManagement/Model.h"
#include "../GLObject/Material.h"
#include "Transform.h"
#include "SceneObject.h"
#include <glm/glm.hpp>

class MeshObject : public SceneObject
{
public:
	MeshObject(Transform* parent, Material* material, Model* model, const std::string& name,
			glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
			glm::vec3 velocity, glm::vec3 angularVelicoty);
	
	MeshObject(Transform* parent, Material* material, Model* model, const std::string& name,
			glm::vec3 position, glm::quat rotation, glm::vec3 scale,
			glm::vec3 velocity, glm::vec3 angularVelicoty);
	
	void update() override;
	
	Model* getModel();
	void setModel(Model* model);
	
	Material* getMaterial();
	void setMaterial(Material* material);
	
	Material* getDrawingMaterial();
	
	glm::vec3 getVelocity() const;
	void setVelocity(glm::vec3 velocity);
	
	glm::vec3 getAngularVelocity() const;
	void setAngularVelocity(glm::vec3 angularVelicoty);
	
	bool getContributeShadows() const;
	
	const Model* getModel() const;
	
	void setContributeShadows(bool contributeShadows);
	
private:
	Material* _material;
	Model* _model;
	
	glm::vec3 _velocity = glm::vec3(0);
	glm::vec3 _angularVelicoty = glm::vec3(0);
	bool _contributeShadows = true;
};
