#include "Transform.h"
#include "../Helper/VectorHelper.h"
#include <stdexcept>
#include <glm/gtc/matrix_inverse.hpp>
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
	
	glm::vec3 worldPos = getWorldPosition();
	glm::quat worldRot = getWorldRotation();
	glm::vec3 worldScale = getWorldScale();
	
	glm::vec3 parentPos = parent->getWorldPosition();
	glm::quat parentRot = parent->getWorldRotation();
	glm::vec3 parentScale = parent->getWorldScale();
	
	glm::quat parentRotConjugate = glm::conjugate(parentRot);
	
	glm::vec3 localPos = (parentRotConjugate *
						  (worldPos - parentPos)) /
						 parentScale;
	glm::quat localRot = parentRotConjugate * worldRot;
	glm::vec3 localScale = glm::conjugate(localRot) * ((localRot * worldScale) / parentScale);
	
	setLocalPosition(localPos);
	setLocalRotation(localRot);
	setLocalScale(localScale);
	
	if (_parent != nullptr)
	{
		VectorHelper::removeAll(_parent->_children, this);
	}
	_parent = parent;
	_parent->_children.push_back(this);
	
	invalidateWorldCache();
}

std::vector<Transform*>& Transform::getChildren()
{
	return _children;
}

void Transform::invalidateLocalCache()
{
	if (_invalidLocalCache) return;
	_invalidLocalCache = true;
	invalidateWorldCache();
}

void Transform::invalidateWorldCache()
{
	if (_invalidWorldCache) return;
	_invalidWorldCache = true;
	
	for (Transform* child : _children)
	{
		child->invalidateWorldCache();
	}
}

glm::vec3 Transform::getLocalPosition()
{
	return _localPosition;
}

glm::vec3 Transform::getWorldPosition()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedWorldPosition;
}

void Transform::setLocalPosition(glm::vec3 position)
{
	if (position == _localPosition) return;
	
	_localPosition = position;
	invalidateLocalCache();
}

glm::quat Transform::getLocalRotation()
{
	return _localRotation;
}

glm::quat Transform::getWorldRotation()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedWorldRotation;
}

void Transform::setLocalRotation(glm::quat rotation)
{
	if (rotation == _localRotation) return;
	
	_localRotation = rotation;
	invalidateLocalCache();
}

glm::vec3 Transform::getLocalScale()
{
	return _localScale;
}

glm::vec3 Transform::getWorldScale()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedWorldScale;
}

void Transform::setLocalScale(glm::vec3 scale)
{
	if (scale == _localScale) return;
	
	_localScale = scale;
	invalidateLocalCache();
}

glm::vec3 Transform::getEulerRotation()
{
	return glm::degrees(glm::eulerAngles(_localRotation));
}

void Transform::setEulerRotation(glm::vec3 eulerRotation)
{
	setLocalRotation(glm::quat(glm::radians(eulerRotation)));
}

const glm::mat4& Transform::getLocalToParentMatrix()
{
	if (_invalidLocalCache)
	{
		recalculateLocalCache();
	}
	
	return _cachedLocalToParentMatrix;
}

const glm::mat4& Transform::getParentToLocalMatrix()
{
	if (_invalidLocalCache)
	{
		recalculateLocalCache();
	}
	
	return _cachedParentToLocalMatrix;
}

const glm::mat4& Transform::getLocalToWorldMatrix()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedLocalToWorldMatrix;
}

const glm::mat4& Transform::getWorldToLocalMatrix()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedWorldToLocalMatrix;
}

Transform::Transform() :
		_owner(nullptr), _parent(nullptr), _localPosition(0), _localRotation(glm::vec3(0)), _localScale(1)
{

}

Transform::~Transform()
{
	if (_parent != nullptr)
		VectorHelper::removeAll(_parent->_children, this);
	
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

Transform::Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) :
		Transform(owner, parent, position, glm::quat(glm::radians(rotation)), scale)
{

}

Transform::Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::quat rotation, glm::vec3 scale) :
		_owner(owner)
{
	setParent(parent);
	setLocalPosition(position);
	setLocalRotation(rotation);
	setLocalScale(scale);
}

glm::vec3 Transform::getForward()
{
	return localToWorldDirection(glm::vec3(1, 0, 0));
}

glm::vec3 Transform::getUp()
{
	return localToWorldDirection(glm::vec3(0, 1, 0));
}

glm::vec3 Transform::getLeft()
{
	return localToWorldDirection(glm::vec3(0, 0, 1));
}

void Transform::recalculateWorldCache()
{
	glm::vec3 localPos = getLocalPosition();
	glm::quat localRot = getLocalRotation();
	glm::vec3 localScale = getLocalScale();
	
	glm::mat4 parentLTW;
	
	glm::vec3 parentPos;
	glm::quat parentRot;
	glm::vec3 parentScale;
	
	if (_parent != nullptr)
	{
		parentLTW = _parent->getLocalToWorldMatrix();
		
		parentPos = _parent->getWorldPosition();
		parentRot = _parent->getWorldRotation();
		parentScale = _parent->getWorldScale();
	}
	else
	{
		parentLTW = glm::mat4(1);
		
		parentPos = glm::vec3(0);
		parentRot = glm::quat(1, 0, 0, 0);
		parentScale = glm::vec3(1);
	}
	
	_cachedLocalToWorldMatrix = parentLTW * getLocalToParentMatrix();
	_cachedWorldToLocalMatrix = glm::affineInverse(_cachedLocalToWorldMatrix);
	
	_cachedWorldPosition = parentPos +
						   parentRot * (localPos * parentScale);
	_cachedWorldRotation = parentRot * localRot;
	_cachedWorldScale = glm::conjugate(localRot) * (parentScale * (localRot * localScale));
	
	_cachedLocalToWorldRotation = glm::toMat3(_cachedWorldRotation);
	_cachedWorldToLocalRotation = glm::affineInverse(_cachedLocalToWorldRotation);
	
	_invalidWorldCache = false;
}

void Transform::recalculateLocalCache()
{
	_cachedLocalToParentMatrix = glm::translate(_localPosition) *
								 glm::toMat4(_localRotation) *
								 glm::scale(_localScale);
	_cachedParentToLocalMatrix = glm::affineInverse(_cachedLocalToParentMatrix);
	
	_cachedLocalToParentRotation = glm::toMat3(_localRotation);
	_cachedParentToLocalRotation = glm::affineInverse(_cachedLocalToParentRotation);
	
	_invalidLocalCache = false;
}

glm::vec3 Transform::localToWorldDirection(glm::vec3 localDir)
{
	return getLocalToWorldRotation() * localDir;
}

glm::vec3 Transform::worldToLocalDirection(glm::vec3 worldDir)
{
	return getWorldToLocalRotation() * worldDir;
}

glm::vec3 Transform::localToParentDirection(glm::vec3 localDir)
{
	return getLocalToParentRotation() * localDir;
}

glm::vec3 Transform::parentToLocalDirection(glm::vec3 worldDir)
{
	return getParentToLocalRotation() * worldDir;
}

const glm::mat3& Transform::getLocalToWorldRotation()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedLocalToWorldRotation;
}

const glm::mat3& Transform::getWorldToLocalRotation()
{
	if (_invalidWorldCache)
	{
		recalculateWorldCache();
	}
	return _cachedWorldToLocalRotation;
}

const glm::mat3& Transform::getLocalToParentRotation()
{
	if (_invalidWorldCache)
	{
		recalculateLocalCache();
	}
	return _cachedLocalToParentRotation;
}

const glm::mat3& Transform::getParentToLocalRotation()
{
	if (_invalidWorldCache)
	{
		recalculateLocalCache();
	}
	return _cachedParentToLocalRotation;
}
