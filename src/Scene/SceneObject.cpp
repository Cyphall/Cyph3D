#include "SceneObject.h"

#include <utility>

SceneObject::SceneObject(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale):
_name(name), _transform(this, parent, position, rotation, scale)
{

}

SceneObject::SceneObject(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale):
_name(name), _transform(this, parent, position, rotation, scale)
{

}

Transform& SceneObject::getTransform()
{
	return _transform;
}

const std::string& SceneObject::getName() const
{
	return _name;
}

void SceneObject::setName(std::string name)
{
	_name = std::move(name);
}

void SceneObject::update()
{

}
