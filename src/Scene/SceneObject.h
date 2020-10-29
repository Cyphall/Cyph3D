#pragma once

#include "Transform.h"
#include <string>

class SceneObject
{
public:
	SceneObject() = delete;
	virtual void update(double deltaTime);
	
	Transform& getTransform();
	const std::string& getName() const;
	void setName(std::string name);
	
protected:
	Transform _transform;
	std::string _name;
	
	SceneObject(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	SceneObject(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale);
};


