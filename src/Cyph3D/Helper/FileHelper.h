#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct FileDialogFilter
{
	const wchar_t* fileTypeDisplayName;
	const wchar_t* fileTypeExtensions;
};

class FileHelper
{
public:
	static std::string readAllText(const std::string& path);
	static std::optional<std::filesystem::path> fileDialogOpen(std::vector<FileDialogFilter> allowedFileTypes, const std::filesystem::path& defaultFolder);
	static std::optional<std::filesystem::path> fileDialogSave(std::vector<FileDialogFilter> allowedFileTypes, const std::filesystem::path& defaultFolder, const std::string& defaultName);
};