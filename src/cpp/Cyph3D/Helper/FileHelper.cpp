#include "FileHelper.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

#if defined(_WIN32)
#	define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#	define GLFW_EXPOSE_NATIVE_COCOA
#else
#	define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <nfd_glfw3.h>

namespace
{
std::filesystem::path rootDirectoryPath = std::filesystem::current_path();
std::filesystem::path assetDirectoryPath = rootDirectoryPath / "assets";

std::filesystem::path cacheRootDirectoryPath = rootDirectoryPath / "cache";
std::filesystem::path cacheAssetDirectoryPath = cacheRootDirectoryPath / "assets";
}

std::string c3d::FileHelper::readAllText(const std::filesystem::path& path)
{
	std::ifstream file = openFileForReading(path);

	size_t fileSize = std::filesystem::file_size(path);
	std::string fileContent(fileSize, '\0');

	file.read(fileContent.data(), fileContent.size());

	return fileContent;
}

std::ifstream c3d::FileHelper::openFileForReading(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (file.fail())
	{
		throw std::system_error(errno, std::iostream_category(), std::format("Cannot open \"{}\" for reading", path.generic_string()));
	}

	return file;
}

std::ofstream c3d::FileHelper::openFileForWriting(const std::filesystem::path& path)
{
	std::ofstream file(path, std::ios::out | std::ios::binary);

	if (file.fail())
	{
		throw std::system_error(errno, std::iostream_category(), std::format("Cannot open \"{}\" for writing", path.generic_string()));
	}

	return file;
}

std::optional<std::filesystem::path> c3d::FileHelper::fileDialogOpen(std::span<const nfdfilteritem_t> allowedFileTypes, const std::filesystem::path& defaultFolder)
{
	std::optional<std::filesystem::path> res;

	nfdwindowhandle_t handle{};
	NFD_GetNativeWindowFromGLFWWindow(Engine::getWindow().getHandle(), &handle);

	NFD::UniquePath dialogPath;
	nfdresult_t result = NFD::OpenDialog(
		dialogPath,
		allowedFileTypes.data(),
		allowedFileTypes.size(),
		std::filesystem::absolute(defaultFolder).generic_string().c_str(),
		handle
	);

	switch (result)
	{
	case NFD_CANCEL:
		break;
	case NFD_OKAY:
		res = dialogPath.get();
		break;
	case NFD_ERROR:
		throw std::runtime_error(NFD::GetError());
	default:
		std::terminate();
	}

	return res;
}

std::optional<std::filesystem::path> c3d::FileHelper::fileDialogSave(std::span<const nfdfilteritem_t> allowedFileTypes, const std::filesystem::path& defaultFolder, const std::string& defaultName)
{
	std::optional<std::filesystem::path> res;

	nfdwindowhandle_t handle{};
	NFD_GetNativeWindowFromGLFWWindow(Engine::getWindow().getHandle(), &handle);

	NFD::UniquePath dialogPath;
	nfdresult_t result = NFD::SaveDialog(
		dialogPath,
		allowedFileTypes.data(),
		allowedFileTypes.size(),
		std::filesystem::absolute(defaultFolder).generic_string().c_str(),
		defaultName.c_str(),
		handle
	);

	switch (result)
	{
	case NFD_CANCEL:
		break;
	case NFD_OKAY:
		res = dialogPath.get();
		break;
	case NFD_ERROR:
		throw std::runtime_error(NFD::GetError());
	default:
		std::terminate();
	}

	return res;
}

const std::filesystem::path& c3d::FileHelper::getRootDirectoryPath()
{
	return rootDirectoryPath;
}

const std::filesystem::path& c3d::FileHelper::getAssetDirectoryPath()
{
	return assetDirectoryPath;
}

const std::filesystem::path& c3d::FileHelper::getCacheRootDirectoryPath()
{
	return cacheRootDirectoryPath;
}

const std::filesystem::path& c3d::FileHelper::getCacheAssetDirectoryPath()
{
	return cacheAssetDirectoryPath;
}

bool c3d::FileHelper::isAssetPath(const std::filesystem::path& path)
{
	std::filesystem::path assetPathCanonial = std::filesystem::weakly_canonical(FileHelper::getAssetDirectoryPath());
	std::filesystem::path pathCanonial = std::filesystem::weakly_canonical(std::filesystem::absolute(path));

	auto it = std::search(pathCanonial.begin(), pathCanonial.end(), assetPathCanonial.begin(), assetPathCanonial.end());

	return it == pathCanonial.begin();
}

void c3d::FileHelper::init()
{
	NFD::Init();
}

void c3d::FileHelper::shutdown()
{
	NFD::Quit();
}