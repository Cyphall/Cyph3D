#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>

class Animator : public Component
{
public:
	explicit Animator(Entity& entity);
	
	glm::vec3 getVelocity() const;
	void setVelocity(glm::vec3 velocity);
	
	glm::vec3 getAngularVelocity() const;
	void setAngularVelocity(glm::vec3 angularVelicoty);
	
	void onUpdate() override;
	void onDrawUi() override;
	
	void duplicate(Entity& targetEntity) const override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	glm::vec3 _velocity = glm::vec3(0);
	glm::vec3 _angularVelicoty = glm::vec3(0);
};