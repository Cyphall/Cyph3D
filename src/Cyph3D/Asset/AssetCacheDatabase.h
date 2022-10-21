#pragma once

#include "Cyph3D/Enums/TextureType.h"

#include <memory>
#include <string>
#include <glad/glad.h>

namespace SQLite
{
	class Database;
}

class AssetCacheDatabase
{
public:
	AssetCacheDatabase();
	~AssetCacheDatabase();

	std::string getImageCachePath(std::string_view path, const GLenum& format);
	std::string getMeshCachePath(std::string_view path);
	
private:
	std::unique_ptr<SQLite::Database> _database;
};