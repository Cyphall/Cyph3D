#pragma once

#include <functional>
#include <imgui.h>
#include <memory>

namespace c3d
{
class UIAssetBrowser
{
public:
	explicit UIAssetBrowser();
	~UIAssetBrowser();

	void draw();

private:
	class Entry;

	void rescan();

	void drawLeftPanel();
	void drawDirectoryNode(const Entry& directory);

	void drawRightPanelEntries();
	void drawRightPanelEntry(const Entry& entry, const char* icon, float& usedWidth);

	std::unique_ptr<Entry> _root;
	const Entry* _currentDirectory = nullptr;
	const Entry* _selectedEntry = nullptr;

	float _size1 = 0;
	float _size2 = 0;
	float _previousWidth = 0;

	std::function<void()> _task;
};
}