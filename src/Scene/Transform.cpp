#include "Transform.h"
#include "../Helper/VectorHelper.h"
#include <stdexcept>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

Transform* Transform::getParent()
{
	return _parent;
}

void Transform::setParent(Transform* parent)
{
	if (parent == _parent) return;
	if (parent == this) throw std::runtime_error("Cannot set Transform's parent to itself");
	if (parent == nullptr) throw std::runtime_error("Cannot remove Transform's parent, only changing it is allowed");
	
	if (_parent != nullptr)
		removeAll(_parent->_children, this);
	_parent = parent;
	_parent->_children.push_back(this);
	
	worldMatrixChanged();
}

std::vector<Transform*>& Transform::getChildren()
{
	return _children;
}

void Transform::matrixChanged()
{
	if (_matrixChanged) return;
	_matrixChanged = true;
	worldMatrixChanged();
}

void Transform::worldMatrixChanged()
{
	if (_worldMatrixChanged) return;
	_worldMatrixChanged = true;
	
	for (Transform* child : _children)
	{
		child->worldMatrixChanged();
	}
}

glm::vec3 Transform::getPosition()
{
	return _position;
}

glm::vec3 Transform::getWorldPosition()
{
	return glm::vec3(getWorldMatrix() * glm::vec4(0, 0, 0, 1));
}

void Transform::setPosition(glm::vec3 position)
{
	if (position == _position) return;
	
	_position = position;
	matrixChanged();
}

glm::quat Transform::getRotation()
{
	return _rotation;
}

glm::quat Transform::getWorldRotation()
{
	// note: theorical, not tested, might not work
	return _parent != nullptr ? _parent->getWorldRotation() * _rotation : _rotation;
}

void Transform::setRotation(glm::quat rotation)
{
	if (rotation == _rotation) return;
	
	_rotation = rotation;
	matrixChanged();
}

glm::vec3 Transform::getScale()
{
	return _scale;
}

glm::vec3 Transform::getEulerRotation()
{
	return glm::degrees(glm::eulerAngles(_rotation));
}

void Transform::setEulerRotation(glm::vec3 eulerRotation)
{
	setRotation(glm::quat(glm::radians(eulerRotation)));
}

glm::vec3 Transform::getWorldScale()
{
	// note: theorical, not tested, might not work
	return _parent != nullptr ? _parent->getWorldScale() * _scale : _scale;
}

void Transform::setScale(glm::vec3 scale)
{
	if (scale == _scale) return;
	
	_scale = scale;
	matrixChanged();
}

const glm::mat4& Transform::getMatrix()
{
	if (_matrixChanged)
	{
		_cachedMatrix = glm::translate(_position) *
				  glm::transpose(glm::toMat4(_rotation)) *
				  glm::scale(_scale);
		_matrixChanged = false;
	}
	
	return _cachedMatrix;
}

const glm::mat4& Transform::getWorldMatrix()
{
	if (_worldMatrixChanged)
	{
		_cachedWorldMatrix = _parent != nullptr ? _parent->getWorldMatrix() * getMatrix() : getMatrix();
		_worldMatrixChanged = false;
	}
	return _cachedWorldMatrix;
}

Transform::Transform():
_owner(nullptr), _parent(nullptr), _position(0), _rotation(glm::vec3(0)), _scale(1)
{

}

Transform::~Transform()
{
	if (_parent != nullptr)
		removeAll(_parent->_children, this);
	
	for (Transform* child : _children)
	{
		child->setParent(_parent);
	}
}

SceneObject* Transform::getOwner()
{
	return _owner;
}

std::unique_ptr<Transform> Transform::createSceneRoot()
{
	return std::unique_ptr<Transform>(new Transform());
}

Transform::Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale):
Transform(owner, parent, position, glm::quat(glm::radians(rotation)), scale)
{

}

Transform::Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::quat rotation, glm::vec3 scale):
_owner(owner), _parent(nullptr), _position(position), _rotation(rotation), _scale(scale)
{
	setParent(parent);
}
