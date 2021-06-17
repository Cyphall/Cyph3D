#include "FileHelper.h"
#include <fstream>
#include <ios>
#include <format>
#include <shobjidl_core.h>
#include <stdlib.h>
#include <filesystem>

std::string FileHelper::readAllText(const std::string& path)
{
	std::ifstream in(path, std::ios::in | std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw std::ios_base::failure(std::format("Could not find file \"{}\"", path));
}

std::optional<std::string> FileHelper::fileDialogOpen(std::vector<FileDialogFilter> allowedFileTypes)
{
	IFileOpenDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	
	if (!SUCCEEDED(hr))
		return std::nullopt;
	
	std::vector<COMDLG_FILTERSPEC> fileTypes;
	fileTypes.reserve(allowedFileTypes.size());
	for (const FileDialogFilter& fileType : allowedFileTypes)
	{
		COMDLG_FILTERSPEC& filter = fileTypes.emplace_back();
		filter.pszName = fileType.fileTypeDisplayName;
		filter.pszSpec = fileType.fileTypeExtensions;
	}
	
	hr = pfd->SetFileTypes(fileTypes.size(), fileTypes.data());
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	IShellItem* scenesFolder;
	std::filesystem::path scenesFolderPath("resources/scenes");
	hr = SHCreateItemFromParsingName(absolute(scenesFolderPath).wstring().c_str(), nullptr, IID_PPV_ARGS(&scenesFolder));
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	hr = pfd->SetDefaultFolder(scenesFolder);
	scenesFolder->Release();
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	hr = pfd->Show(NULL);
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	IShellItem* item;
	hr = pfd->GetResult(&item);
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	LPWSTR filePathStrRaw;
	item->GetDisplayName(SIGDN_FILESYSPATH, &filePathStrRaw);
	pfd->Release();
	
	std::filesystem::path filePath(filePathStrRaw);
	CoTaskMemFree(filePathStrRaw);
	
	return filePath.generic_string();
}

std::optional<std::string> FileHelper::fileDialogSave(std::vector<FileDialogFilter> allowedFileTypes, const std::string& defaultName)
{
	IFileSaveDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	
	if (!SUCCEEDED(hr))
		return std::nullopt;
	
	std::vector<COMDLG_FILTERSPEC> fileTypes;
	fileTypes.reserve(allowedFileTypes.size());
	for (const FileDialogFilter& fileType : allowedFileTypes)
	{
		COMDLG_FILTERSPEC& filter = fileTypes.emplace_back();
		filter.pszName = fileType.fileTypeDisplayName;
		filter.pszSpec = fileType.fileTypeExtensions;
	}
	
	hr = pfd->SetFileTypes(fileTypes.size(), fileTypes.data());
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	IShellItem* defaultFile;
	std::filesystem::path defaultFilePath(std::format("resources/scenes/{}.json", defaultName));
	hr = SHCreateItemFromParsingName(absolute(defaultFilePath).wstring().c_str(), nullptr, IID_PPV_ARGS(&defaultFile));
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	hr = pfd->SetSaveAsItem(defaultFile);
	defaultFile->Release();
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	hr = pfd->Show(NULL);
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	IShellItem* item;
	hr = pfd->GetResult(&item);
	
	if (!SUCCEEDED(hr))
	{
		pfd->Release();
		return std::nullopt;
	}
	
	LPWSTR filePathStrRaw;
	item->GetDisplayName(SIGDN_FILESYSPATH, &filePathStrRaw);
	pfd->Release();
	
	std::filesystem::path filePath(filePathStrRaw);
	CoTaskMemFree(filePathStrRaw);
	
	return filePath.generic_string();
}
