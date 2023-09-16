#pragma once

#include <imgui.h>
#include <optional>
#include <string_view>

class ImGuiHelper
{
public:
	static bool AssetInputWidget(const std::string* currentAssetPath, const char* label, const char* dragDropId, std::optional<std::string_view>& result);
	static void BeginGroupPanel(const char* label = nullptr);
	static void EndGroupPanel();
	static void MoveCursorPos(const glm::vec2& offset);
	static void TextCentered(const char* text);
};