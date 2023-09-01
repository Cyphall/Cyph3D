#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>
#include <sigslot/signal.hpp>

class Entity;

class Transform
{
public:
	Transform(Entity* owner, Transform* parent);
	Transform(const Transform& other) = delete;
	Transform(Transform&& other) = delete;
	
	~Transform();
	
	Transform* getParent() const;
	void setParent(Transform* parent);
	
	std::vector<Transform*>& getChildren();
	const std::vector<Transform*>& getChildren() const;
	
	glm::vec3 getLocalPosition() const;
	glm::vec3 getWorldPosition() const;
	void setLocalPosition(glm::vec3 position);
	
	glm::quat getLocalRotation() const;
	glm::quat getWorldRotation() const;
	void setLocalRotation(glm::quat rotation);
	
	glm::vec3 getLocalScale() const;
	glm::vec3 getWorldScale() const;
	void setLocalScale(glm::vec3 scale);
	
	glm::vec3 getEulerLocalRotation() const;
	glm::vec3 getEulerWorldRotation() const;
	void setEulerLocalRotation(glm::vec3 eulerRotation);
	
	glm::vec3 getRight() const;
	glm::vec3 getLeft() const;
	glm::vec3 getUp() const;
	glm::vec3 getDown() const;
	glm::vec3 getBackward() const;
	glm::vec3 getForward() const;
	
	const glm::mat4& getLocalToWorldMatrix() const;
	const glm::mat4& getWorldToLocalMatrix() const;
	
	const glm::mat4& getLocalToParentMatrix() const;
	const glm::mat4& getParentToLocalMatrix() const;
	
	glm::mat4 calcCustomLocalToWorldMatrix(bool translate, bool rotate, bool scale) const;
	glm::mat4 calcCustomWorldToLocalMatrix(bool translate, bool rotate, bool scale) const;
	
	glm::mat4 calcCustomLocalToParentMatrix(bool translate, bool rotate, bool scale) const;
	glm::mat4 calcCustomParentToLocalMatrix(bool translate, bool rotate, bool scale) const;
	
	glm::vec3 localToWorldDirection(glm::vec3 localDir) const;
	glm::vec3 worldToLocalDirection(glm::vec3 worldDir) const;
	
	glm::vec3 localToParentDirection(glm::vec3 localDir) const;
	glm::vec3 parentToLocalDirection(glm::vec3 worldDir) const;
	
	glm::vec3 localToWorldPosition(glm::vec3 localDir) const;
	glm::vec3 worldToLocalPosition(glm::vec3 worldDir) const;
	
	glm::vec3 localToParentPosition(glm::vec3 localDir) const;
	glm::vec3 parentToLocalPosition(glm::vec3 worldDir) const;
	
	Entity* getOwner();
	
	sigslot::signal<>& getChangedSignal();
	
	static std::unique_ptr<Transform> createSceneRoot();
	
private:
	// mandatory, base data. also used for getLocal[Position/Rotation/Scale]()
	glm::vec3 _localPosition = glm::vec3(0);
	glm::quat _localRotation = glm::quat(1, 0, 0, 0);
	glm::vec3 _localScale = glm::vec3(1);
	
	// used for getWorld[Position/Rotation/Scale]()
	mutable glm::vec3 _cachedWorldPosition;
	mutable glm::quat _cachedWorldRotation;
	mutable glm::vec3 _cachedWorldScale;
	
	// used for localToWorldMatrix() and worldToLocalMatrix()
	mutable glm::mat4 _cachedLocalToWorldMatrix;
	mutable glm::mat4 _cachedWorldToLocalMatrix;
	
	// used to cache parent<=>local matrices
	mutable glm::mat4 _cachedLocalToParentMatrix;
	mutable glm::mat4 _cachedParentToLocalMatrix;
	
	mutable bool _invalidLocalCache = true;
	mutable bool _invalidWorldCache = true;
	
	Transform* _parent = nullptr;
	std::vector<Transform*> _children;
	
	Entity* _owner;
	
	sigslot::signal<> _changed;
	
	// Scene root constructor
	Transform();
	
	void invalidateLocalCache() const;
	void invalidateWorldCache() const;
	
	void recalculateWorldCache() const;
	void recalculateLocalCache() const;
};

