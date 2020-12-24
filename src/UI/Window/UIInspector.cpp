#include "UIInspector.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Scene/SceneObject.h"
#include "../../Scene/MeshObject.h"
#include "../../Scene/Light.h"
#include "../../Scene/DirectionalLight.h"
#include "../../Scene/PointLight.h"
#include "../../Rendering/Renderer.h"
#include "../../Engine.h"

std::any UIInspector::_selected;
bool UIInspector::_showRawQuaternion = false;
bool UIInspector::_currentlyClicking = false;
glm::dvec2 UIInspector::_clickPos;

std::any UIInspector::getSelected()
{
	return _selected;
}

void UIInspector::setSelected(std::any selected)
{
	_selected = selected;
}

bool UIInspector::getShowRawQuaternion()
{
	return _showRawQuaternion;
}

void UIInspector::setShowRawQuaternion(bool value)
{
	_showRawQuaternion = value;
}

void UIInspector::show()
{
	ImGui::SetNextWindowSize(glm::vec2(400, Engine::getWindow().getSize().y - Engine::getWindow().getSize().y / 2));
	ImGui::SetNextWindowPos(glm::vec2(0, Engine::getWindow().getSize().y / 2));
	
	if (!ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) return;
	
	if (_selected.has_value())
	{
		if (_selected.type() == typeid(Transform*))
		{
			showSceneObject(std::any_cast<Transform*>(_selected)->getOwner());
		}
		
		if (_selected.type() == typeid(Material*))
		{
			showMaterial(std::any_cast<Material*>(_selected));
		}
	}
	
	ImGui::End();
	
	if (!_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
	{
		_currentlyClicking = true;
		_clickPos = Engine::getWindow().getCursorPos();
	}
	
	if (_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		_currentlyClicking = false;
		if (glm::distance(_clickPos, Engine::getWindow().getCursorPos()) < 5)
		{
			GLint previousFramebuffer;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
			
			GLuint framebuffer;
			glCreateFramebuffers(1, &framebuffer);
			glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, Engine::getRenderer().getTextures()["gbuffer_objectIndex"]->getHandle(), 0);
			glNamedFramebufferReadBuffer(framebuffer, GL_COLOR_ATTACHMENT0);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			
			_clickPos.y = Engine::getWindow().getSize().y - _clickPos.y;
			
			int objectIndex;
			glReadPixels(_clickPos.x, _clickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &objectIndex);
			
			glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
			glDeleteFramebuffers(1, &framebuffer);
			
			if (objectIndex != -1)
			{
				setSelected(&Engine::getRenderer().getRegistry().meshObjects[objectIndex]->getTransform());
			}
		}
	}
}

void UIInspector::showSceneObject(SceneObject* selected)
{
	std::string imGuiName = selected->getName();
	if (ImGui::InputText("Name", &imGuiName))
	{
		selected->setName(imGuiName);
	}
	
	ImGui::Text("Transform");
	
	glm::vec3 imGuiPosition = selected->getTransform().getPosition();
	if (ImGui::DragFloat3("Position", glm::value_ptr(imGuiPosition), 0.01f))
	{
		selected->getTransform().setPosition(imGuiPosition);
	}
	
	if (_showRawQuaternion)
	{
		glm::quat imGuiRotation = selected->getTransform().getRotation();
		if (ImGui::DragFloat4("Quaternion", glm::value_ptr(imGuiRotation), 0.01f))
		{
			selected->getTransform().setRotation(imGuiRotation);
		}
	}
	else
	{
		glm::vec3 imGuiRotation = selected->getTransform().getEulerRotation();
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(imGuiRotation), 0.01f))
		{
			selected->getTransform().setEulerRotation(imGuiRotation);
		}
	}
	
	glm::vec3 imGuiScale = selected->getTransform().getScale();
	if (ImGui::DragFloat3("Scale", glm::value_ptr(imGuiScale), 0.01f))
	{
		selected->getTransform().setScale(imGuiScale);
	}
	
	ImGui::Separator();
	
	ImGui::Text("Properties");
	
	MeshObject* meshObject = dynamic_cast<MeshObject*>(selected);
	if (meshObject != nullptr)
	{
		showMeshObject(meshObject);
	}
	
	Light* light = dynamic_cast<Light*>(selected);
	if (light != nullptr)
	{
		showLight(light);
	}
}

void UIInspector::showMeshObject(MeshObject* meshObject)
{
	glm::vec3 imGuiVelocity = meshObject->getVelocity();
	if (ImGui::DragFloat3("Velocity", glm::value_ptr(imGuiVelocity), 0.01f))
	{
		meshObject->setVelocity(imGuiVelocity);
	}
	
	glm::vec3 imGuiAngularVelocity = meshObject->getAngularVelocity();
	if (ImGui::DragFloat3("Angular Velocity", glm::value_ptr(imGuiAngularVelocity), 0.01f))
	{
		meshObject->setAngularVelocity(imGuiAngularVelocity);
	}
	
	Model* model = meshObject->getModel();
	std::string modelName = model != nullptr ? model->getName() : "None";
	ImGui::InputText("Mesh", &modelName, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshDragDrop");
		if (payload)
		{
			meshObject->setModel(Engine::getScene().getRM().requestModel(*(*static_cast<const std::string**>(payload->Data))));
		}
		ImGui::EndDragDropTarget();
	}
	
	Material* material = meshObject->getMaterial();
	std::string materialName = material != nullptr ? material->getName() : "None";
	ImGui::InputText("Material", &materialName, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MaterialDragDrop");
		if (payload)
		{
			meshObject->setMaterial(Engine::getScene().getRM().requestMaterial(*(*static_cast<const std::string**>(payload->Data))));
		}
		ImGui::EndDragDropTarget();
	}
	
	bool contributeShadows = meshObject->getContributeShadows();
	if (ImGui::Checkbox("Contribute Shadows", &contributeShadows))
	{
		meshObject->setContributeShadows(contributeShadows);
	}
}

void UIInspector::showLight(Light* light)
{
	glm::vec3 imGuiSrgbColor = light->getSrgbColor();
	if (ImGui::ColorEdit3("Color", glm::value_ptr(imGuiSrgbColor)))
	{
		light->setSrgbColor(imGuiSrgbColor);
	}
	
	float intensity = light->getIntensity();
	if (ImGui::DragFloat("Intensity", &intensity, 0.01f))
	{
		light->setIntensity(intensity);
	}
	
	DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(light);
	if (directionalLight != nullptr)
	{
		showDirectionalLight(directionalLight);
	}
	
	PointLight* pointLight = dynamic_cast<PointLight*>(light);
	if (pointLight != nullptr)
	{
		showPointLight(pointLight);
	}
}

void UIInspector::showDirectionalLight(DirectionalLight* light)
{
	bool castShadows = light->getCastShadows();
	if (ImGui::Checkbox("Cast Shadows", &castShadows))
	{
		light->setCastShadows(castShadows);
	}
	
	const char* resolutions[] = {"256", "512", "1024", "2048", "4096", "8192"};
	int currIndex = glm::log2(light->getResolution() / 256);
	if (ImGui::BeginCombo("Shadow Resolution", resolutions[currIndex]))
	{
		for (int i = 0; i < 6; i++)
		{
			bool selected = i == currIndex;
			if (ImGui::Selectable(resolutions[i], selected))
			{
				currIndex = i;
				light->setResolution(256 * glm::pow(2, i));
			}
			
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		
		ImGui::EndCombo();
	}
}

void UIInspector::showPointLight(PointLight* light)
{
	bool castShadows = light->getCastShadows();
	if (ImGui::Checkbox("Cast Shadows", &castShadows))
	{
		light->setCastShadows(castShadows);
	}
	
	const char* resolutions[] = {"256", "512", "1024", "2048", "4096", "8192"};
	int currIndex = glm::log2(light->getResolution() / 256);
	if (ImGui::BeginCombo("Shadow Resolution", resolutions[currIndex]))
	{
		for (int i = 0; i < 6; i++)
		{
			bool selected = i == currIndex;
			if (ImGui::Selectable(resolutions[i], selected))
			{
				currIndex = i;
				light->setResolution(256 * glm::pow(2, i));
			}
			
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		
		ImGui::EndCombo();
	}
}

void UIInspector::showMaterial(Material* material)
{

}
