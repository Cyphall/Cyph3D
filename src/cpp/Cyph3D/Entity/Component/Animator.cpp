#include "Animator.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Entity/Entity.h>
#include <Cyph3D/ObjectSerialization.h>
#include <Cyph3D/UI/Window/UIMisc.h>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

const char* c3d::Animator::identifier = "Animator";

c3d::Animator::Animator(Entity& entity):
	Component(entity)
{}

glm::vec3 c3d::Animator::getVelocity() const
{
	return _velocity;
}

void c3d::Animator::setVelocity(glm::vec3 velocity)
{
	_velocity = velocity;

	_changed();
}

glm::vec3 c3d::Animator::getAngularVelocity() const
{
	return _angularVelicoty;
}

void c3d::Animator::setAngularVelocity(glm::vec3 angularVelicoty)
{
	_angularVelicoty = angularVelicoty;

	_changed();
}

void c3d::Animator::onUpdate()
{
	if (!UIMisc::isSimulationEnabled())
	{
		return;
	}

	double deltaTime = Engine::getTimer().deltaTime();

	Transform& transform = getTransform();

	glm::vec3 newPosition = transform.localToParentPosition(_velocity * static_cast<float>(deltaTime));
	transform.setLocalPosition(newPosition);

	glm::vec3 rotationOffset = _angularVelicoty * static_cast<float>(deltaTime);

	glm::quat rotation = transform.getLocalRotation();
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.x), glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.y), glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, glm::radians(rotationOffset.z), glm::vec3(0, 0, 1));
	transform.setLocalRotation(rotation);
}

c3d::ObjectSerialization c3d::Animator::serialize() const
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

void c3d::Animator::deserialize(const ObjectSerialization& serialization)
{
	switch (serialization.version)
	{
	case 1:
		deserializeFromVersion1(serialization.data);
		break;
	default:
		throw;
	}
}

void c3d::Animator::onDrawUi()
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

const char* c3d::Animator::getIdentifier() const
{
	return identifier;
}

void c3d::Animator::duplicate(Entity& targetEntity) const
{
	Animator& newComponent = targetEntity.addComponent<Animator>();
	newComponent.setVelocity(getVelocity());
	newComponent.setAngularVelocity(getAngularVelocity());
}

void c3d::Animator::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	setVelocity(glm::make_vec3(jsonRoot["velocity"].get<std::vector<float>>().data()));
	setAngularVelocity(glm::make_vec3(jsonRoot["angular_velocity"].get<std::vector<float>>().data()));
}