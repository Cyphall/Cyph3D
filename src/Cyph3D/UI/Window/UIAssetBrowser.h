#pragma once

#include <filesystem>
#include <string>
#include <glm/glm.hpp>
#include <imgui.h>
#include <functional>
#include <memory>

class UIAssetBrowser
{
public:
	explicit UIAssetBrowser(ImFont* bigFont);
	~UIAssetBrowser();

	void draw();

private:
	class Entry;

	void rescan();

	void drawLeftPanel();
	void drawDirectoryNode(const Entry& directory);

	void drawRightPanel();
	bool drawRightPanelEntry(const std::string& id, const char* icon, const std::string& name, float& usedWidth);

	ImFont* _bigFont;

	std::unique_ptr<Entry> _root;
	const Entry* _currentDirectory = nullptr;

	float _size1 = 0;
	float _size2 = 0;
	float _previousWidth = 0;

	std::function<void(void)> _task;
};