#include "UIAssetBrowser.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"

#include <imgui_internal.h>
#include <algorithm>
#include <set>

enum class EntryType
{
	Directory,
	
	Unknown,
	
	Image,
	Mesh,
	
	Material,
	Skybox,
	Scene
};

class UIAssetBrowser::Entry
{
public:
	struct EntryCompare
	{
		bool operator()(const std::unique_ptr<Entry>& left, const std::unique_ptr<Entry>& right) const
		{
			if (left->type() == right->type())
			{
				return left->name() < right->name();
			}
			else
			{
				return left->type() < right->type();
			}
		}
	};
	
	Entry(const std::filesystem::path& assetPath, EntryType type):
		_assetPath(assetPath.generic_string()),
		_name(assetPath.filename().generic_string()),
		_truncatedName(truncate(_name, 90 * Engine::getWindow().getPixelScale())),
		_type(type)
	{
		if (type == EntryType::Directory)
		{
			for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(FileHelper::getAssetDirectoryPath() / assetPath))
			{
				EntryType entryType;
				
				if (entry.is_directory())
				{
					entryType = EntryType::Directory;
				}
				else
				{
					std::string extension = entry.path().extension().generic_string();
					std::transform(
						extension.begin(), extension.end(),
						extension.begin(),
						[](char c){ return std::tolower(c); });
					
					if (extension == ".png" || extension == ".jpg")
					{
						entryType = EntryType::Image;
					}
					else if (extension == ".obj")
					{
						entryType = EntryType::Mesh;
					}
					else if (extension == ".c3dmaterial")
					{
						entryType = EntryType::Material;
					}
					else if (extension == ".c3dskybox")
					{
						entryType = EntryType::Skybox;
					}
					else if (extension == ".c3dscene")
					{
						entryType = EntryType::Scene;
					}
					else
					{
						entryType = EntryType::Unknown;
					}
				}

				_entries.emplace(std::make_unique<Entry>(
					std::filesystem::relative(entry.path(), FileHelper::getAssetDirectoryPath()),
					entryType));
			}
		}
	}

	Entry(const Entry& other) = delete;
	Entry& operator=(const Entry& other) = delete;

	const std::string& assetPath() const
	{
		return _assetPath;
	}

	const std::string& truncatedName() const
	{
		return _truncatedName;
	}

	const std::string& name() const
	{
		return _name;
	}
	
	const EntryType& type() const
	{
		return _type;
	}

	std::set<std::unique_ptr<Entry>, EntryCompare>& entries()
	{
		return _entries;
	}

	const std::set<std::unique_ptr<Entry>, EntryCompare>& entries() const
	{
		return _entries;
	}

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

private:
	std::string _assetPath;
	std::string _name;
	std::string _truncatedName;
	EntryType _type;
	std::set<std::unique_ptr<Entry>, EntryCompare> _entries;
};

UIAssetBrowser::UIAssetBrowser(ImFont* bigFont):
	_bigFont(bigFont),
	_size1(250.0f * Engine::getWindow().getPixelScale()),
	_previousWidth(_size1)
{

}

UIAssetBrowser::~UIAssetBrowser()
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
		bool anyWidgetClicked = drawRightPanel();
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !anyWidgetClicked)
		{
			_selectedEntry = nullptr;
		}
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
	_root = std::make_unique<UIAssetBrowser::Entry>(FileHelper::getAssetDirectoryPath(), EntryType::Directory);
	_currentDirectory = _root.get();
	_selectedEntry = nullptr;
}

void UIAssetBrowser::drawLeftPanel()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 14 * Engine::getWindow().getPixelScale());
	drawDirectoryNode(*_root);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void UIAssetBrowser::drawDirectoryNode(const UIAssetBrowser::Entry& directory)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	if (&directory == _root.get())
		flags |= ImGuiTreeNodeFlags_DefaultOpen;

	if (&directory == _currentDirectory)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (directory.entries().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	bool opened = ImGui::TreeNodeEx(directory.assetPath().c_str(), flags, "\uF07B %s", directory.name().c_str());

	//Select the item on click
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		_currentDirectory = &directory;
	}

	//Draw item children if the item is opened
	if (opened)
	{
		for (const std::unique_ptr<Entry>& entry : directory.entries())
		{
			drawDirectoryNode(*entry);
		}

		ImGui::TreePop();
	}
}

void UIAssetBrowser::drawRightPanelEntry(const Entry& entry, const char* icon, float& usedWidth, bool& clicked, bool& doubleClicked)
{
	clicked = false;
	doubleClicked = false;
	
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

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

	ImGui::InvisibleButton(entry.assetPath().c_str(), entrySize);
	
	if (&entry == _selectedEntry)
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(100, 100, 100, 255));
		drawList->AddRectFilled(entryOrigin, entryOrigin + entrySize, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBgHovered]));
		ImGui::PopStyleColor();
	}
	else if (ImGui::IsItemHovered())
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(80, 80, 80, 255));
		drawList->AddRectFilled(entryOrigin, entryOrigin + entrySize, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBgHovered]));
		ImGui::PopStyleColor();
	}

	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseDoubleClicked(0))
		{
			doubleClicked = true;
		}
		else if (ImGui::IsMouseClicked(0))
		{
			clicked = true;
		}
	}
	
	glm::vec3 iconColor = glm::vec4(style.Colors[ImGuiCol_Text]);
	iconColor -= 0.5f;
	iconColor *= 0.7f;
	iconColor += 0.5f;

	ImGui::PushFont(_bigFont);
	glm::vec2 iconSize = ImGui::CalcTextSize(icon);
	drawList->AddText(contentOrigin + iconOffset + (iconAvailableSize / 2.0f - iconSize / 2.0f), ImGui::GetColorU32(glm::vec4(iconColor, style.Colors[ImGuiCol_Text].w)), icon);
	ImGui::PopFont();

	glm::vec2 nameSize = ImGui::CalcTextSize(entry.truncatedName().c_str());
	drawList->AddText(contentOrigin + nameOffset + (nameAvailableSize / 2.0f - nameSize / 2.0f), ImGui::GetColorU32(style.Colors[ImGuiCol_Text]), entry.truncatedName().c_str());

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
}

bool UIAssetBrowser::drawRightPanel()
{
	float usedWidth = 0;
	bool anyWidgetClicked = false;
	for (const std::unique_ptr<Entry>& entry : _currentDirectory->entries())
	{
		const char* icon;
		const char* dragDropId;
		switch (entry->type())
		{
			case EntryType::Directory:
				icon = "\uF07B";
				dragDropId = nullptr;
				break;
			case EntryType::Unknown:
				icon = "\uF15B";
				dragDropId = nullptr;
				break;
			case EntryType::Image:
				icon = "\uF03E";
				dragDropId = "asset_image";
				break;
			case EntryType::Mesh:
				icon = "\uF1B2";
				dragDropId = "asset_mesh";
				break;
			case EntryType::Material:
				icon = "\uF43C";
				dragDropId = "asset_material";
				break;
			case EntryType::Skybox:
				icon = "\uE209";
				dragDropId = "asset_skybox";
				break;
			case EntryType::Scene:
				icon = "\uE52f";
				dragDropId = nullptr;
				break;
			default:
				throw;
		}
		
		bool clicked;
		bool doubleClicked;
		drawRightPanelEntry(*entry, icon, usedWidth, clicked, doubleClicked);

		if (dragDropId != nullptr)
		{
			if (ImGui::BeginDragDropSource())
			{
				ImGui::TextUnformatted(entry->assetPath().c_str());

				const std::string* pathPtr = &entry->assetPath();

				ImGui::SetDragDropPayload(dragDropId, &pathPtr, sizeof(const std::string*));
				ImGui::EndDragDropSource();
			}
		}
		
		if (clicked)
		{
			_selectedEntry = entry.get();
			anyWidgetClicked = true;
		}
		else if (doubleClicked)
		{
			switch (entry->type())
			{
				case EntryType::Scene:
					Scene::load(entry->assetPath());
					break;
				case EntryType::Directory:
					_task = [this, &entry]()
					{
						this->_currentDirectory = entry.get();
					};
					break;
				default:
					break;
			}
		}
	}
	
	return anyWidgetClicked;
}