#pragma once

#include <any>
#include <glm/glm.hpp>

class SceneObject;
class MeshObject;
class Light;
class DirectionalLight;
class PointLight;
class Material;

class UIInspector
{
public:
	static void show();
	
	static std::any getSelected();
	static void setSelected(std::any selected);
	
	static bool getShowRawQuaternion();
	static void setShowRawQuaternion(bool value);

private:
	static std::any _selected;
	static bool _showRawQuaternion;
	static bool _currentlyClicking;
	static glm::dvec2 _clickPos;
	
	static void showSceneObject(SceneObject* selected);
	static void showMeshObject(MeshObject* meshObject);
	static void showLight(Light* light);
	static void showDirectionalLight(DirectionalLight* light);
	static void showPointLight(PointLight* light);
	static void showMaterial(Material* material);
};
