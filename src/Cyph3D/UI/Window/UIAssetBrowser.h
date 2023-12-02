#pragma once

#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>
#include <string>

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

	void drawRightPanelEntries();
	void drawRightPanelEntry(const Entry& entry, const char* icon, float& usedWidth);

	ImFont* _bigFont;

	std::unique_ptr<Entry> _root;
	const Entry* _currentDirectory = nullptr;
	const Entry* _selectedEntry = nullptr;

	float _size1 = 0;
	float _size2 = 0;
	float _previousWidth = 0;

	std::function<void(void)> _task;
};