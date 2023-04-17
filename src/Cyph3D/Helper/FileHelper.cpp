#include "FileHelper.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"

#include <GLFW/glfw3.h>
#undef APIENTRY
#include <ShObjIdl_core.h>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

static std::filesystem::path rootDirectoryPath = std::filesystem::current_path();
static std::filesystem::path assetDirectoryPath = rootDirectoryPath / "resources";

static std::filesystem::path cacheRootDirectoryPath = rootDirectoryPath / "cache";
static std::filesystem::path cacheAssetDirectoryPath = cacheRootDirectoryPath / "assets";

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

std::optional<std::filesystem::path> FileHelper::fileDialogOpen(const std::vector<FileDialogFilter>& allowedFileTypes, const std::filesystem::path& defaultFolder)
{
	std::optional<std::filesystem::path> res;
	
	IFileOpenDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		nullptr,
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
				hr = pfd->SetFolder(defaultFolderItem);
				
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

std::optional<std::filesystem::path> FileHelper::fileDialogSave(const std::vector<FileDialogFilter>& allowedFileTypes, const std::filesystem::path& defaultFolder, const std::string& defaultName)
{
	std::optional<std::filesystem::path> res;
	
	IFileSaveDialog* pfd;
	
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog,
		nullptr,
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
				hr = pfd->SetFolder(defaultFolderItem);
				
				if (SUCCEEDED(hr))
				{
					hr = pfd->SetDefaultExtension(fileTypes[0].pszName);
					
					if (SUCCEEDED(hr))
					{
						std::filesystem::path defaultFileName(defaultName);
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
				}
				defaultFolderItem->Release();
			}
		}
		pfd->Release();
	}
	
	return res;
}

const std::filesystem::path& FileHelper::getRootDirectoryPath()
{
	return rootDirectoryPath;
}

const std::filesystem::path& FileHelper::getAssetDirectoryPath()
{
	return assetDirectoryPath;
}

const std::filesystem::path& FileHelper::getCacheRootDirectoryPath()
{
	return cacheRootDirectoryPath;
}

const std::filesystem::path& FileHelper::getCacheAssetDirectoryPath()
{
	return cacheAssetDirectoryPath;
}

bool FileHelper::isAssetPath(const std::filesystem::path& path)
{
	std::filesystem::path assetPathCanonial = std::filesystem::weakly_canonical(FileHelper::getAssetDirectoryPath());
	std::filesystem::path pathCanonial = std::filesystem::weakly_canonical(std::filesystem::absolute(path));

	auto it = std::search(pathCanonial.begin(), pathCanonial.end(), assetPathCanonial.begin(), assetPathCanonial.end());

	return it == pathCanonial.begin();
}

void FileHelper::init()
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
}

void FileHelper::shutdown()
{
	CoUninitialize();
}
