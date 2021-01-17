#include "MeshObject.h"

MeshObject::MeshObject(Transform* parent, Material* material, Model* model, const std::string& name,
					   glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
					   glm::vec3 velocity, glm::vec3 angularVelicoty):
					   SceneObject(parent, name, position, rotation, scale),
					   _material(material), _model(model), _velocity(velocity), _angularVelicoty(angularVelicoty)
{

}

MeshObject::MeshObject(Transform* parent, Material* material, Model* model, const std::string& name,
					   glm::vec3 position, glm::quat rotation, glm::vec3 scale,
					   glm::vec3 velocity, glm::vec3 angularVelicoty):
					   SceneObject(parent, name, position, rotation, scale),
					   _material(material), _model(model), _velocity(velocity), _angularVelicoty(angularVelicoty)
{

}

void MeshObject::update(double deltaTime)
{
	_transform.setPosition(_transform.getPosition() + _velocity * (float)deltaTime);
	
	glm::vec3 rotationOffset = _angularVelicoty * (float)deltaTime;
	
	glm::quat rotation = _transform.getRotation();
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.x), glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.y), glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.z), glm::vec3(0, 0, 1));
	_transform.setRotation(rotation);
}

Material* MeshObject::getMaterial()
{
	return _material;
}

Material* MeshObject::getDrawingMaterial()
{
	return _material != nullptr ? _material : Material::getMissing();
}

void MeshObject::setMaterial(Material* material)
{
	_material = material;
}

Model* MeshObject::getModel()
{
	return _model;
}

void MeshObject::setModel(Model* model)
{
	_model = model;
}

bool MeshObject::getContributeShadows() const
{
	return _contributeShadows;
}

void MeshObject::setContributeShadows(bool contributeShadows)
{
	_contributeShadows = contributeShadows;
}

glm::vec3 MeshObject::getVelocity() const
{
	return _velocity;
}

void MeshObject::setVelocity(glm::vec3 velocity)
{
	_velocity = velocity;
}

glm::vec3 MeshObject::getAngularVelocity() const
{
	return _angularVelicoty;
}

void MeshObject::setAngularVelocity(glm::vec3 angularVelicoty)
{
	_angularVelicoty = angularVelicoty;
}

const Model* MeshObject::getModel() const
{
	return _model;
}
