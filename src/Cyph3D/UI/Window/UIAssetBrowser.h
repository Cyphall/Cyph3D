#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <imgui.h>
#include <functional>
#include <memory>

class UIAssetBrowser
{
public:
	explicit UIAssetBrowser(ImFont* bigFont);

	void draw();

private:
	struct File;
	struct Directory;

	enum class FileType
	{
		Unknown,
		Material,
		Model,
		Skybox,
		Scene
	};

	struct File
	{
		std::string path;
		std::string name;
		std::string truncatedName;
		FileType type = FileType::Unknown;
	};

	struct Directory
	{
		std::string path;
		std::string name;
		std::string truncatedName;
		std::vector<File> files;
		std::vector<std::unique_ptr<Directory>> subdirectories;
	};

	void rescan();

	std::unique_ptr<Directory> build(const std::filesystem::path& path);

	void drawLeftPanel();
	void drawDirectoryNode(const Directory* directory);

	void drawRightPanel();
	bool drawRightPanelEntry(const std::string& id, const char* icon, const std::string& name, float& usedWidth);

	ImFont* _bigFont;

	std::unique_ptr<Directory> _root;
	const Directory* _selected = nullptr;

	float _size1 = 250;
	float _size2 = 0;
	float _previousWidth = 250;

	std::function<void(void)> _task;
};