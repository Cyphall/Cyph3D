#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <fstream>

struct FileDialogFilter
{
	const wchar_t* fileTypeDisplayName;
	const wchar_t* fileTypeExtensions;
};

class FileHelper
{
public:
	static void init();
	static void shutdown();
	
	static std::string readAllText(const std::string& path);
	
	static std::ifstream openFileForReading(const std::filesystem::path& path);
	static std::ofstream openFileForWriting(const std::filesystem::path& path);
	
	static std::optional<std::filesystem::path> fileDialogOpen(const std::vector<FileDialogFilter>& allowedFileTypes, const std::filesystem::path& defaultFolder);
	static std::optional<std::filesystem::path> fileDialogSave(const std::vector<FileDialogFilter>& allowedFileTypes, const std::filesystem::path& defaultFolder, const std::string& defaultName);
	
	static const std::filesystem::path& getRootDirectoryPath();
	static const std::filesystem::path& getAssetDirectoryPath();
	static const std::filesystem::path& getCacheRootDirectoryPath();
	static const std::filesystem::path& getCacheAssetDirectoryPath();

	static bool isAssetPath(const std::filesystem::path& path);
	
	template<typename T>
	static void read(std::ifstream& stream, T* data)
	{
		stream.read(reinterpret_cast<char*>(data), sizeof(T));
	}

	template<typename T>
	static void read(std::ifstream& stream, std::vector<T>& data)
	{
		size_t dataSize;
		read(stream, &dataSize);

		data.resize(dataSize);
		
		stream.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
	}

	template<typename T, size_t Size>
	static void read(std::ifstream& stream, std::array<T, Size>& data)
	{
		stream.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
	}

	template<typename T>
	static void write(std::ofstream& stream, const T* data)
	{
		stream.write(reinterpret_cast<const char*>(data), sizeof(T));
	}

	template<typename T>
	static void write(std::ofstream& stream, const std::vector<T>& data)
	{
		size_t dataSize = data.size();
		write(stream, &dataSize);
		
		stream.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
	}
	
	template<typename T, size_t Size>
	static void write(std::ofstream& stream, const std::array<T, Size>& data)
	{
		stream.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
	}
};