#include "Cyph3D/Entity/Component/Animator.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

const char* Animator::identifier = "Animator";

Animator::Animator(Entity& entity):
Component(entity)
{}

glm::vec3 Animator::getVelocity() const
{
	return _velocity;
}

void Animator::setVelocity(glm::vec3 velocity)
{
	_velocity = velocity;
}

glm::vec3 Animator::getAngularVelocity() const
{
	return _angularVelicoty;
}

void Animator::setAngularVelocity(glm::vec3 angularVelicoty)
{
	_angularVelicoty = angularVelicoty;
}

void Animator::onUpdate()
{
	double deltaTime = Engine::getTimer().deltaTime();
	
	Transform& transform = getTransform();
	
	glm::vec3 velocity = transform.localToParentDirection(_velocity);
	
	transform.setLocalPosition(transform.getLocalPosition() + velocity * (float) deltaTime);
	
	glm::vec3 rotationOffset = _angularVelicoty * (float)deltaTime;
	
	glm::quat rotation = transform.getLocalRotation();
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.x), glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.y), glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.z), glm::vec3(0, 0, 1));
	transform.setLocalRotation(rotation);
}

ObjectSerialization Animator::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 1;
	serialization.identifier = getIdentifier();
	
	glm::vec3 velocity = getVelocity();
	serialization.data["velocity"] = {velocity.x, velocity.y, velocity.z};
	glm::vec3 angularVelocity = getAngularVelocity();
	serialization.data["angular_velocity"] = {angularVelocity.x, angularVelocity.y, angularVelocity.z};
	
	return serialization;
}

void Animator::deserialize(const ObjectSerialization& serialization)
{
	setVelocity(glm::make_vec3(serialization.data["velocity"].get<std::vector<float>>().data()));
	setAngularVelocity(glm::make_vec3(serialization.data["angular_velocity"].get<std::vector<float>>().data()));
}

void Animator::onDrawUi()
{
	glm::vec3 velocity = getVelocity();
	if (ImGui::DragFloat3("Velocity", glm::value_ptr(velocity), 0.01f))
	{
		setVelocity(velocity);
	}
	
	glm::vec3 angularVelocity = getAngularVelocity();
	if (ImGui::DragFloat3("Angular Velocity", glm::value_ptr(angularVelocity), 0.01f))
	{
		setAngularVelocity(angularVelocity);
	}
}

const char* Animator::getIdentifier() const
{
	return identifier;
}

void Animator::duplicate(Entity& targetEntity) const
{
	Animator& newComponent = targetEntity.addComponent<Animator>();
	newComponent.setVelocity(getVelocity());
	newComponent.setAngularVelocity(getAngularVelocity());
}