#include "UIResourceExplorer.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../../Window.h"
#include "../../Engine.h"
#include <magic_enum.hpp>
#include <algorithm>
#include <filesystem>
#include "../../Helper/StringHelper.h"

std::vector<std::string> UIResourceExplorer::_meshes;
std::vector<std::string> UIResourceExplorer::_materials;
std::vector<std::string> UIResourceExplorer::_skyboxes;

ResourceType UIResourceExplorer::_currentResourceType;

void UIResourceExplorer::init()
{
	rescanFiles();
}

void UIResourceExplorer::show()
{
	if (!ImGui::Begin("Resources", nullptr))
	{
		ImGui::End();
		return;
	}
	
	ImGui::BeginChild("type", glm::vec2(100, 0));
	for (auto& [resourceType, resourceTypeName] : magic_enum::enum_entries<ResourceType>())
	{
		if (ImGui::Selectable(std::string(resourceTypeName).c_str(), resourceType == _currentResourceType))
		{
			_currentResourceType = resourceType;
		}
	}
	ImGui::EndChild();
	
	ImGui::SameLine();
	
	ImGui::BeginGroup();
	
	glm::vec2 buttonSize = glm::vec2(ImGui::CalcTextSize("Refresh")) + glm::vec2(ImGui::GetStyle().FramePadding) * 2.0f;
	ImGui::Spacing();
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonSize.x);
	if (ImGui::Button("Refresh"))
	{
		rescanFiles();
	}
	
	ImGui::BeginChild("list", glm::vec2(0), true);
	switch (_currentResourceType)
	{
		case ResourceType::Mesh:
			for (const std::string& mesh : _meshes)
			{
				ImGui::Selectable(mesh.c_str());
				
				if (ImGui::BeginDragDropSource())
				{
					ImGui::Text("%s", mesh.c_str());
					
					const std::string* meshPtr = &mesh;
					ImGui::SetDragDropPayload("MeshDragDrop", &meshPtr, sizeof(std::string*));
					ImGui::EndDragDropSource();
				}
			}
			break;
		case ResourceType::Material:
			for (const std::string& material : _materials)
			{
				ImGui::Selectable(material.c_str());
				
				if (ImGui::BeginDragDropSource())
				{
					ImGui::Text("%s", material.c_str());
					
					const std::string* materialPtr = &material;
					ImGui::SetDragDropPayload("MaterialDragDrop", &materialPtr, sizeof(std::string*));
					ImGui::EndDragDropSource();
				}
			}
			break;
		case ResourceType::Skybox:
			for (const std::string& skybox : _skyboxes)
			{
				ImGui::Selectable(skybox.c_str());
				
				if (ImGui::BeginDragDropSource())
				{
					ImGui::Text("%s", skybox.c_str());
					
					const std::string* skyboxPtr = &skybox;
					ImGui::SetDragDropPayload("SkyboxDragDrop", &skyboxPtr, sizeof(std::string*));
					ImGui::EndDragDropSource();
				}
			}
			break;
	}
	ImGui::EndChild();
	
	ImGui::EndGroup();
	
	ImGui::End();
}

void UIResourceExplorer::rescanFiles()
{
	_meshes.clear();
	findMeshes("resources/meshes/");
	std::sort(_meshes.begin(), _meshes.end());
	
	_materials.clear();
	findMaterial("resources/materials/");
	std::sort(_materials.begin(), _materials.end());
	
	_skyboxes.clear();
	findSkyboxes("resources/skyboxes/");
	std::sort(_skyboxes.begin(), _skyboxes.end());
}

void UIResourceExplorer::findMeshes(const std::string& path)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
	{
		if (entry.is_directory()) continue;
		
		std::string name = std::filesystem::path(entry.path()).replace_extension().generic_string();
		StringHelper::remove(name, path.c_str());
		_meshes.push_back(name);
	}
}

void UIResourceExplorer::findMaterial(const std::string& path)
{
	_materials.push_back("internal/Default Material");
	
	std::filesystem::recursive_directory_iterator it(path);
	for (const auto& entry : it)
	{
		if (entry.is_directory())
		{
			if (entry.path().filename() == "internal")
				it.disable_recursion_pending();
			continue;
		}
		
		if (entry.path().filename() == "material.json")
		{
			std::string name = entry.path().parent_path().generic_string();
			StringHelper::remove(name, path.c_str());
			_materials.push_back(name);
		}
	}
}

void UIResourceExplorer::findSkyboxes(const std::string& path)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
	{
		if (entry.is_directory()) continue;
		
		if (entry.path().filename() == "skybox.json")
		{
			std::string name = entry.path().parent_path().generic_string();
			StringHelper::remove(name, path.c_str());
			_skyboxes.push_back(name);
		}
	}
}
