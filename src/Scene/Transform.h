#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <memory>

class SceneObject;

class Transform
{
public:
	Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	Transform(SceneObject* owner, Transform* parent, glm::vec3 position, glm::quat rotation, glm::vec3 scale);
	Transform(const Transform& other) = delete;
	Transform(Transform&& other) = delete;
	
	~Transform();
	
	Transform* getParent();
	void setParent(Transform* parent);
	std::vector<Transform*>& getChildren();
	
	glm::vec3 getLocalPosition();
	glm::vec3 getWorldPosition();
	void setLocalPosition(glm::vec3 position);
	
	glm::quat getLocalRotation();
	glm::quat getWorldRotation();
	void setLocalRotation(glm::quat rotation);
	
	glm::vec3 getLocalScale();
	glm::vec3 getWorldScale();
	void setLocalScale(glm::vec3 scale);
	
	glm::vec3 getEulerRotation();
	void setEulerRotation(glm::vec3 eulerRotation);
	
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getLeft();
	
	const glm::mat4& getLocalToWorldMatrix();
	const glm::mat4& getWorldToLocalMatrix();
	
	const glm::mat4& getLocalToParentMatrix();
	const glm::mat4& getParentToLocalMatrix();
	
	glm::vec3 localToWorldDirection(glm::vec3 localDir);
	glm::vec3 worldToLocalDirection(glm::vec3 worldDir);
	
	glm::vec3 localToParentDirection(glm::vec3 localDir);
	glm::vec3 parentToLocalDirection(glm::vec3 worldDir);
	
	SceneObject* getOwner();
	
	static std::unique_ptr<Transform> createSceneRoot();
	
private:
	// mandatory, base data. also used for getLocal[Position/Rotation/Scale]()
	glm::vec3 _localPosition = glm::vec3(0);
	glm::quat _localRotation = glm::quat(1, 0, 0, 0);
	glm::vec3 _localScale = glm::vec3(1);
	
	// used for getWorld[Position/Rotation/Scale]()
	glm::vec3 _cachedWorldPosition;
	glm::quat _cachedWorldRotation;
	glm::vec3 _cachedWorldScale;
	
	// used for localToWorldDirection() and worldToLocalDirection()
	glm::mat3 _cachedLocalToWorldRotation;
	glm::mat3 _cachedWorldToLocalRotation;
	
	// used for localToParentDirection() and parentToLocalDirection()
	glm::mat3 _cachedLocalToParentRotation;
	glm::mat3 _cachedParentToLocalRotation;
	
	// used for localToWorldMatrix() and worldToLocalMatrix()
	glm::mat4 _cachedLocalToWorldMatrix;
	glm::mat4 _cachedWorldToLocalMatrix;
	
	// used to cache parent<=>local matrices
	glm::mat4 _cachedLocalToParentMatrix;
	glm::mat4 _cachedParentToLocalMatrix;
	
	bool _invalidLocalCache = true;
	bool _invalidWorldCache = true;
	
	Transform* _parent = nullptr;
	std::vector<Transform*> _children;
	
	SceneObject* _owner;
	
	// Scene root constructor
	Transform();
	
	void invalidateLocalCache();
	void invalidateWorldCache();
	
	void recalculateWorldCache();
	void recalculateLocalCache();
	
	const glm::mat3& getLocalToWorldRotation();
	const glm::mat3& getWorldToLocalRotation();
	const glm::mat3& getLocalToParentRotation();
	const glm::mat3& getParentToLocalRotation();
};


