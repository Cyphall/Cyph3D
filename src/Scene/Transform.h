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
	
	glm::vec3 getPosition();
	glm::vec3 getWorldPosition();
	void setPosition(glm::vec3 position);
	
	glm::quat getRotation();
	glm::quat getWorldRotation();
	void setRotation(glm::quat rotation);
	
	glm::vec3 getScale();
	glm::vec3 getWorldScale();
	void setScale(glm::vec3 scale);
	
	glm::vec3 getEulerRotation();
	void setEulerRotation(glm::vec3 eulerRotation);
	
	const glm::mat4& getMatrix();
	const glm::mat4& getWorldMatrix();
	
	SceneObject* getOwner();
	
	static std::unique_ptr<Transform> createSceneRoot();
	
private:
	glm::vec3 _position;
	glm::quat _rotation;
	glm::vec3 _scale;
	
	glm::mat4 _cachedWorldMatrix = glm::mat4();
	glm::mat4 _cachedMatrix = glm::mat4();
	
	bool _matrixChanged = true;
	bool _worldMatrixChanged = true;
	
	Transform* _parent;
	std::vector<Transform*> _children;
	
	SceneObject* _owner;
	
	// Scene root constructor
	Transform();
	
	void matrixChanged();
	void worldMatrixChanged();
};


