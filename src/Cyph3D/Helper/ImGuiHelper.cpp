#include "ImGuiHelper.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"

#include <format>
#include <imgui_internal.h>
#include <stack>

bool ImGuiHelper::AssetInputWidget(const std::string* currentAssetPath, const char* label, const char* dragDropId, std::optional<std::string_view>& result)
{
	ImGui::PushID(label);

	bool assetChanged = false;

	float textboxWidth = ImGui::CalcItemWidth() - ImGui::GetFrameHeight();

	ImGui::BeginGroup();

	const char* assetPath = currentAssetPath != nullptr ? currentAssetPath->c_str() : "None";
	size_t assetPathSize = currentAssetPath != nullptr ? currentAssetPath->size() + 1 : 5;

	ImGui::SetNextItemWidth(textboxWidth);
	// Field is read-only anyway, we can safely remove the const from assetName
	ImGui::InputText(std::format("###{}", label).c_str(), const_cast<char*>(assetPath), assetPathSize, ImGuiInputTextFlags_ReadOnly);

	ImGui::SameLine(0.0f, 0.0f);

	if (currentAssetPath == nullptr)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("\uF00D", ImVec2(ImGui::GetFrameHeight(), 0)))
	{
		assetChanged = true;
	}

	if (currentAssetPath == nullptr)
	{
		ImGui::EndDisabled();
	}

	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragDropId);
		if (payload)
		{
			result = *(*static_cast<const std::string**>(payload->Data));
			assetChanged = true;
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::TextUnformatted(label);

	ImGui::PopID();

	return assetChanged;
}

struct GroupPanelInfo
{
	glm::vec2 initialCursorPos;
	const char* label;
	glm::vec2 labelSize;
};

static std::stack<GroupPanelInfo> groupPanelInfoStack;

void ImGuiHelper::BeginGroupPanel(const char* label)
{
	ImGuiStyle& style = ImGui::GetStyle();
	GroupPanelInfo groupPanelInfo{};

	groupPanelInfo.label = label;
	groupPanelInfo.labelSize = groupPanelInfo.label != nullptr ? ImGui::CalcTextSize(groupPanelInfo.label) : ImVec2(0.0f, 0.0f);

	if (groupPanelInfo.label != nullptr)
	{
		ImGui::PushID(groupPanelInfo.label);
	}

	ImGuiHelper::MoveCursorPos(glm::vec2(0, groupPanelInfo.labelSize.y / 2.0f));

	groupPanelInfo.initialCursorPos = ImGui::GetCursorScreenPos();
	groupPanelInfoStack.push(groupPanelInfo);

	ImGuiHelper::MoveCursorPos(glm::vec2(style.WindowPadding) + glm::vec2(0, groupPanelInfo.labelSize.y / 2.0f));

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->ContentRegionRect.Max.x -= style.WindowPadding.x;
	window->WorkRect.Max.x -= style.WindowPadding.x;
	window->InnerRect.Max.x -= style.WindowPadding.x;

	ImGui::BeginGroup();
}

void ImGuiHelper::EndGroupPanel()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	GroupPanelInfo groupPanelInfo = groupPanelInfoStack.top();
	groupPanelInfoStack.pop();

	const float textPadding = 6.0f;

	ImGui::EndGroup();

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->ContentRegionRect.Max.x += style.WindowPadding.x;
	window->WorkRect.Max.x += style.WindowPadding.x;
	window->InnerRect.Max.x += style.WindowPadding.x;

	glm::vec2 labelCenter = groupPanelInfo.initialCursorPos;
	labelCenter.x += (15.0f + textPadding) * Engine::getWindow().getPixelScale() + groupPanelInfo.labelSize.x / 2.0f;

	glm::vec2 borderTopLeft;
	borderTopLeft.x = groupPanelInfo.initialCursorPos.x;
	borderTopLeft.y = groupPanelInfo.initialCursorPos.y;

	glm::vec2 borderBottomRight;
	borderBottomRight.x = ImGui::GetContentRegionMaxAbs().x;
	borderBottomRight.y = groupPanelInfo.initialCursorPos.y + ImGui::GetItemRectSize().y + style.WindowPadding.y * 2.0f + groupPanelInfo.labelSize.y / 2.0f;

	drawList->AddRect(borderTopLeft, borderBottomRight, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Border]), 3.0f);

	if (groupPanelInfo.label != nullptr)
	{
		glm::vec2 patchTopLeft;
		patchTopLeft.x = labelCenter.x - groupPanelInfo.labelSize.x / 2.0f - textPadding * Engine::getWindow().getPixelScale();
		patchTopLeft.y = labelCenter.y - groupPanelInfo.labelSize.y / 2.0f;

		glm::vec2 patchBottomRight;
		patchBottomRight.x = labelCenter.x + groupPanelInfo.labelSize.x / 2.0f + textPadding * Engine::getWindow().getPixelScale();
		patchBottomRight.y = labelCenter.y + groupPanelInfo.labelSize.y / 2.0f;

		drawList->AddRectFilled(patchTopLeft, patchBottomRight, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_WindowBg]));
		drawList->AddText(labelCenter - groupPanelInfo.labelSize / 2.0f, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]), groupPanelInfo.label);
	}

	ImGui::SetCursorScreenPos(groupPanelInfo.initialCursorPos);
	ImGui::Dummy({0, borderBottomRight.y - borderTopLeft.y});

	if (groupPanelInfo.label != nullptr)
	{
		ImGui::PopID();
	}
}

void ImGuiHelper::MoveCursorPos(const glm::vec2& offset)
{
	ImGui::SetCursorPos(glm::vec2(ImGui::GetCursorPos()) + offset);
}

void ImGuiHelper::TextCentered(const char* text)
{
	float textWidth = ImGui::CalcTextSize(text).x;
	float textOffset = std::max(ImGui::GetContentRegionAvail().x / 2.0f - textWidth / 2.0f, 0.0f);
	ImGuiHelper::MoveCursorPos({textOffset, 0.0f});
	ImGui::TextWrapped("%s", text);
}