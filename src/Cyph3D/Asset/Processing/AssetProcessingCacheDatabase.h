#pragma once

#include "Cyph3D/Asset/Processing/ImageData.h"

#include <memory>
#include <string>

namespace SQLite
{
	class Database;
}

class AssetProcessingCacheDatabase
{
public:
	AssetProcessingCacheDatabase();
	~AssetProcessingCacheDatabase();

	std::string getImageCachePath(std::string_view path, ImageType type);
	std::string getMeshCachePath(std::string_view path);
	
private:
	std::unique_ptr<SQLite::Database> _database;
};