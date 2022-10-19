#include "UIAssetBrowser.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"

#include <imgui_internal.h>
#include <algorithm>

static std::string truncate(const std::string& string, float maxWidth)
{
	if (ImGui::CalcTextSize(string.c_str()).x > maxWidth)
	{
		std::vector<char> buffer(string.size() + 3, 0);
		std::fill(buffer.begin(), buffer.begin() + 3, '.');
		std::copy(string.begin(), string.end(), buffer.begin() + 3);

		int min = 0;
		int max = buffer.size();
		while (max - min > 1)
		{
			int middle = (min + max) / 2;
			float width = ImGui::CalcTextSize(buffer.data(), buffer.data() + middle).x;
			if (width > maxWidth)
			{
				max = middle;
			}
			else
			{
				min = middle;
			}
		}
		return string.substr(0, min - 3) + "...";
	}
	else
	{
		return string;
	}
}

UIAssetBrowser::UIAssetBrowser(ImFont* bigFont):
	_bigFont(bigFont),
	_size1(250.0f * Engine::getWindow().getPixelScale()),
	_previousWidth(_size1)
{

}

void UIAssetBrowser::draw()
{
	if (!_root)
	{
		rescan();
	}

	if (ImGui::Begin("Asset Browser"))
	{
		if (ImGui::Button("Refresh"))
		{
			rescan();
		}

		float currentWidth = ImGui::GetContentRegionAvail().x;
		if (_previousWidth != currentWidth)
		{
			float diffWidth = currentWidth - _previousWidth;
			_size2 += diffWidth;
			_previousWidth = currentWidth;
		}

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImRect boundingBox;
		boundingBox.Min = glm::vec2(window->DC.CursorPos) + glm::vec2(_size1, 0.0f);
		boundingBox.Max = glm::vec2(boundingBox.Min) + glm::vec2(ImGui::CalcItemSize({2.0f, -1.0f}, 0.0f, 0.0f));
		ImGui::SplitterBehavior(boundingBox, window->GetID("asset_browser_splitter"), ImGuiAxis_X, &_size1, &_size2, 0, 0, 4.0f, 0.04f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginChild("asset_browser_left_panel", ImVec2(_size1, 0), true);
		drawLeftPanel();
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::SameLine(0.0f, 2.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(4, 12) * Engine::getWindow().getPixelScale());
		ImGui::BeginChild("asset_browser_right_panel", ImVec2(-FLT_MIN, 0), true);
		drawRightPanel();
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	ImGui::End();

	if (_task)
	{
		_task();
		_task = {};
	}
}

void UIAssetBrowser::rescan()
{
	_root.reset();
	_root = build(FileHelper::getAssetDirectoryPath());
	_selected = _root.get();
}

std::unique_ptr<UIAssetBrowser::Directory> UIAssetBrowser::build(const std::filesystem::path& path)
{
	std::unique_ptr<Directory> directory = std::make_unique<UIAssetBrowser::Directory>();
	directory->name = path.filename().generic_string();
	directory->truncatedName = truncate(directory->name, 90 * Engine::getWindow().getPixelScale());
	directory->path = std::filesystem::relative(path, FileHelper::getAssetDirectoryPath()).generic_string();

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			directory->subdirectories.push_back(build(entry.path()));
		}
		else
		{
			File& file = directory->files.emplace_back();
			file.name = entry.path().filename().generic_string();
			file.truncatedName = truncate(file.name, 90 * Engine::getWindow().getPixelScale());
			file.path = std::filesystem::relative(entry.path(), FileHelper::getAssetDirectoryPath()).generic_string();

			std::string extension = entry.path().extension().generic_string();
			std::transform(
				extension.begin(), extension.end(),
				extension.begin(),
				[](char c){ return std::tolower(c); });
			if (extension == ".c3dmaterial")
			{
				file.type = FileType::Material;
			}
			else if (extension == ".obj")
			{
				file.type = FileType::Model;
			}
			else if (extension == ".c3dskybox")
			{
				file.type = FileType::Skybox;
			}
			else if (extension == ".c3dscene")
			{
				file.type = FileType::Scene;
			}
		}
	}

	return directory;
}

void UIAssetBrowser::drawLeftPanel()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 14 * Engine::getWindow().getPixelScale());
	drawDirectoryNode(_root.get());
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void UIAssetBrowser::drawDirectoryNode(const UIAssetBrowser::Directory* directory)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	if (directory == _root.get())
		flags |= ImGuiTreeNodeFlags_DefaultOpen;

	if (directory == _selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (directory->subdirectories.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	bool opened = ImGui::TreeNodeEx(directory->path.c_str(), flags, "\uF07B %s", directory->name.c_str());

	//Select the item on click
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		_selected = directory;
	}

	//Draw item children if the item is opened
	if (opened)
	{
		for (const std::unique_ptr<Directory>& subdirectory : directory->subdirectories)
		{
			drawDirectoryNode(subdirectory.get());
		}

		ImGui::TreePop();
	}
}

bool UIAssetBrowser::drawRightPanelEntry(const std::string& id, const char* icon, const std::string& name, float& usedWidth)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGuiStyle& style = ImGui::GetStyle();

	glm::vec2 entryOrigin = ImGui::GetCursorScreenPos();
	glm::vec2 contentOrigin = entryOrigin + glm::vec2(style.FramePadding);

	glm::vec2 contentSize(90.0f * Engine::getWindow().getPixelScale(), _bigFont->FontSize + style.ItemInnerSpacing.y + ImGui::GetFontSize());
	glm::vec2 entrySize = contentSize + glm::vec2(style.FramePadding) * 2.0f;

	glm::vec2 iconOffset(0, 0);
	glm::vec2 iconAvailableSize(contentSize.x, _bigFont->FontSize);

	glm::vec2 nameOffset(0, _bigFont->FontSize + style.ItemInnerSpacing.y);
	glm::vec2 nameAvailableSize(contentSize.x, ImGui::GetFontSize());

	ImGui::InvisibleButton(id.c_str(), entrySize);

	bool doubleClicked = false;
	if (ImGui::IsItemHovered())
	{
		doubleClicked = ImGui::IsMouseDoubleClicked(0);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(80, 80, 80, 255));
		drawList->AddRectFilled(entryOrigin, entryOrigin + entrySize, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBgHovered]));
		ImGui::PopStyleColor();
	}
	
	glm::vec3 iconColor = glm::vec4(style.Colors[ImGuiCol_Text]);
	iconColor -= 0.5f;
	iconColor *= 0.7f;
	iconColor += 0.5f;

	ImGui::PushFont(_bigFont);
	glm::vec2 iconSize = ImGui::CalcTextSize(icon);
	drawList->AddText(contentOrigin + iconOffset + (iconAvailableSize / 2.0f - iconSize / 2.0f), ImGui::GetColorU32(glm::vec4(iconColor, style.Colors[ImGuiCol_Text].w)), icon);
	ImGui::PopFont();

	glm::vec2 nameSize = ImGui::CalcTextSize(name.c_str());
	drawList->AddText(contentOrigin + nameOffset + (nameAvailableSize / 2.0f - nameSize / 2.0f), ImGui::GetColorU32(style.Colors[ImGuiCol_Text]), name.c_str());

	usedWidth += entrySize.x + style.ItemSpacing.x;

	float remainingWidth = ImGui::GetWindowContentRegionMax().x - usedWidth;
	if (remainingWidth >= entrySize.x + style.ItemSpacing.x)
	{
		ImGui::SameLine();
	}
	else
	{
		usedWidth = 0.0f;
	}

	return doubleClicked;
}

void UIAssetBrowser::drawRightPanel()
{
	float usedWidth = 0;

	for (const std::unique_ptr<Directory>& subdirectory : _selected->subdirectories)
	{
		if (drawRightPanelEntry(subdirectory->path, "\uF07B", subdirectory->truncatedName, usedWidth))
		{
			_task = [this, &subdirectory]()
			{
				this->_selected = subdirectory.get();
			};
		}
	}

	for (const File& file : _selected->files)
	{
		const char* icon = "\uF15B";
		const char* dragDropId = nullptr;
		switch (file.type)
		{
			case FileType::Material:
				icon = "\uF43C";
				dragDropId = "asset_material";
				break;
			case FileType::Model:
				icon = "\uF1B2";
				dragDropId = "asset_model";
				break;
			case FileType::Skybox:
				icon = "\uE209";
				dragDropId = "asset_skybox";
				break;
			case FileType::Scene:
				icon = "\uE52f";
				break;
			default:
				break;
		}
		
		bool doubleClicked = drawRightPanelEntry(file.path, icon, file.truncatedName, usedWidth);

		if (dragDropId != nullptr)
		{
			if (ImGui::BeginDragDropSource())
			{
				float dragDropUsedWidth = 0;
				drawRightPanelEntry(file.path, icon, file.truncatedName, dragDropUsedWidth);

				const std::string* pathPtr = &file.path;

				ImGui::SetDragDropPayload(dragDropId, &pathPtr, sizeof(const std::string*));
				ImGui::EndDragDropSource();
			}
		}

		if (doubleClicked)
		{
			switch (file.type)
			{
				case FileType::Scene:
					Scene::load(file.path);
					break;
				default:
					break;
			}
		}
	}
}