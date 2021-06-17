#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

struct FileDialogFilter
{
	const wchar_t* fileTypeDisplayName;
	const wchar_t* fileTypeExtensions;
};

class FileHelper
{
public:
	static std::string readAllText(const std::string& path);
	static std::optional<std::string> fileDialogOpen(std::vector<FileDialogFilter> allowedFileTypes, const std::string& defaultDirectory);
	static std::optional<std::string> fileDialogSave(std::vector<FileDialogFilter> allowedFileTypes, const std::string& defaultDirectory, const std::string& defaultName);
};
