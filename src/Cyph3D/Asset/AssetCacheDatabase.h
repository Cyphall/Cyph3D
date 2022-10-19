#pragma once

#include "Cyph3D/Enums/TextureType.h"

#include <memory>
#include <crossguid/guid.hpp>
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

	xg::Guid getImageCacheGuid(std::string_view path, const GLenum& format);
	xg::Guid getMeshCacheGuid(std::string_view path);
	
private:
	std::unique_ptr<SQLite::Database> _database;
};