#pragma once

#include "Cyph3D/Asset/Processor/ImageData.h"

#include <memory>
#include <string>

namespace SQLite
{
	class Database;
}

class AssetCacheDatabase
{
public:
	AssetCacheDatabase();
	~AssetCacheDatabase();

	std::string getImageCachePath(std::string_view path, ImageType type);
	std::string getMeshCachePath(std::string_view path);
	
private:
	std::unique_ptr<SQLite::Database> _database;
};