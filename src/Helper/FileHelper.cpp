#include "FileHelper.h"
#include "../Window.h"
#include "../Engine.h"
#include <fstream>
#include <ios>
#include <format>
#include <shobjidl_core.h>
#include <stdlib.h>
#include <filesystem>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

std::optional<std::filesystem::path> FileHelper::fileDialogOpen(std::vector<FileDialogFilter> allowedFileTypes, const std::filesystem::path& defaultFolder)
{
	std::optional<std::filesystem::path> res;
	
	IFileOpenDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	
	if (SUCCEEDED(hr))
	{
		std::vector<COMDLG_FILTERSPEC> fileTypes;
		fileTypes.reserve(allowedFileTypes.size());
		for (const FileDialogFilter& fileType : allowedFileTypes)
		{
			COMDLG_FILTERSPEC& filter = fileTypes.emplace_back();
			filter.pszName = fileType.fileTypeDisplayName;
			filter.pszSpec = fileType.fileTypeExtensions;
		}
		
		hr = pfd->SetFileTypes(fileTypes.size(), fileTypes.data());
		
		if (SUCCEEDED(hr))
		{
			IShellItem* defaultFolderItem;
			hr = SHCreateItemFromParsingName(absolute(defaultFolder).wstring().c_str(), nullptr, IID_PPV_ARGS(&defaultFolderItem));
			
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetDefaultFolder(defaultFolderItem);
				
				if (SUCCEEDED(hr))
				{
					hr = pfd->Show(glfwGetWin32Window(Engine::getWindow().getHandle()));
					
					if (SUCCEEDED(hr))
					{
						IShellItem* item;
						hr = pfd->GetResult(&item);
						
						if (SUCCEEDED(hr))
						{
							LPWSTR filePathStrRaw;
							item->GetDisplayName(SIGDN_FILESYSPATH, &filePathStrRaw);
							item->Release();
							
							res = filePathStrRaw;
							CoTaskMemFree(filePathStrRaw);
						}
					}
				}
				defaultFolderItem->Release();
			}
		}
		pfd->Release();
	}
	
	return res;
}

std::optional<std::filesystem::path> FileHelper::fileDialogSave(std::vector<FileDialogFilter> allowedFileTypes, const std::filesystem::path& defaultFolder, const std::string& defaultName)
{
	std::optional<std::filesystem::path> res;
	
	IFileSaveDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	
	if (SUCCEEDED(hr))
	{
		std::vector<COMDLG_FILTERSPEC> fileTypes;
		fileTypes.reserve(allowedFileTypes.size());
		for (const FileDialogFilter& fileType : allowedFileTypes)
		{
			COMDLG_FILTERSPEC& filter = fileTypes.emplace_back();
			filter.pszName = fileType.fileTypeDisplayName;
			filter.pszSpec = fileType.fileTypeExtensions;
		}
		
		hr = pfd->SetFileTypes(fileTypes.size(), fileTypes.data());
		
		if (SUCCEEDED(hr))
		{
			IShellItem* defaultFolderItem;
			hr = SHCreateItemFromParsingName(absolute(defaultFolder).wstring().c_str(), nullptr, IID_PPV_ARGS(&defaultFolderItem));
			
			if (SUCCEEDED(hr))
			{
				hr = pfd->SetDefaultFolder(defaultFolderItem);
				
				if (SUCCEEDED(hr))
				{
					std::filesystem::path defaultFileName(std::format("{}.json", defaultName));
					hr = pfd->SetFileName(defaultFileName.wstring().c_str());
					
					if (SUCCEEDED(hr))
					{
						hr = pfd->Show(glfwGetWin32Window(Engine::getWindow().getHandle()));
						
						if (SUCCEEDED(hr))
						{
							IShellItem* item;
							hr = pfd->GetResult(&item);
							
							if (SUCCEEDED(hr))
							{
								LPWSTR filePathStrRaw;
								item->GetDisplayName(SIGDN_FILESYSPATH, &filePathStrRaw);
								item->Release();
								
								res = filePathStrRaw;
								CoTaskMemFree(filePathStrRaw);
							}
						}
					}
				}
				defaultFolderItem->Release();
			}
		}
		pfd->Release();
	}
	
	return res;
}
