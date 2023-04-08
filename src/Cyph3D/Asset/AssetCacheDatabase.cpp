#include "AssetCacheDatabase.h"

#include "Cyph3D/Helper/FileHelper.h"

#include <SQLiteCpp/Database.h>
#include <sqlite3.h>
#include <crossguid/guid.hpp>
#include <filesystem>

static xg::Guid columnToGuid(const SQLite::Column& column)
{
	if (column.getBytes() != 16)
	{
		throw;
	}

	std::array<uint8_t, 16> guidBytes{};
	const uint8_t* blob = static_cast<const uint8_t*>(column.getBlob());
	std::copy(blob, blob + 16, guidBytes.begin());

	return xg::Guid(guidBytes);
}

AssetCacheDatabase::AssetCacheDatabase()
{
	std::filesystem::path databaseFilePath = FileHelper::getCacheRootDirectoryPath() / "assets/cache_database.sqlite";

	std::filesystem::create_directories(databaseFilePath.parent_path());
	_database = std::make_unique<SQLite::Database>(databaseFilePath.generic_string(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXRESCODE);

	_database->exec(
		"CREATE TABLE IF NOT EXISTS Image\n"
		"(\n"
		"	guid BINARY(16) NOT NULL PRIMARY KEY,\n"
		"	path TEXT NOT NULL,\n"
		"	lastWriteTime BIGINT NOT NULL,\n"
		"	type INT NOT NULL,\n"
		"	UNIQUE(path, lastWriteTime, type)\n"
		") WITHOUT ROWID;");

	_database->exec(
		"CREATE TABLE IF NOT EXISTS Mesh\n"
		"(\n"
		"	guid BINARY(16) NOT NULL PRIMARY KEY,\n"
		"	path TEXT NOT NULL,\n"
		"	lastWriteTime BIGINT NOT NULL,\n"
		"	UNIQUE(path, lastWriteTime)\n"
		") WITHOUT ROWID;");
}

AssetCacheDatabase::~AssetCacheDatabase()
{}

std::string AssetCacheDatabase::getImageCachePath(std::string_view path, ImageType type)
{
	int64_t currentLastWriteTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(FileHelper::getAssetDirectoryPath() / path).time_since_epoch()).count();

	SQLite::Statement selectQuery(*_database,
		"SELECT guid, lastWriteTime FROM Image\n"
		"WHERE path=? AND type=?;");

	selectQuery.bind(1, path.data(), path.size());
	selectQuery.bind(2, static_cast<uint32_t>(type));

	xg::Guid guid;
	int64_t lastWriteTime;

	if (selectQuery.executeStep())
	{
		guid = columnToGuid(selectQuery.getColumn(0));
		lastWriteTime = selectQuery.getColumn(1).getInt64();
	}
	else
	{
		guid = xg::newGuid();
		lastWriteTime = currentLastWriteTime;

		SQLite::Statement insertQuery(*_database,
			"INSERT INTO Image\n"
			"VALUES(?, ?, ?, ?);");

		insertQuery.bind(1, guid.bytes().data(), guid.bytes().size());
		insertQuery.bind(2, path.data(), path.size());
		insertQuery.bind(3, lastWriteTime);
		insertQuery.bind(4, static_cast<uint32_t>(type));

		insertQuery.exec();
	}
	
	std::string cachePath = std::format("images/{}.c3dcache", guid.str());
	
	if (currentLastWriteTime != lastWriteTime)
	{
		std::filesystem::remove(FileHelper::getCacheAssetDirectoryPath() / cachePath);

		SQLite::Statement updateQuery(*_database,
			"UPDATE Image\n"
			"SET lastWriteTime=?\n"
			"WHERE guid=?;");

		updateQuery.bind(1, currentLastWriteTime);
		updateQuery.bind(2, guid.bytes().data(), guid.bytes().size());

		updateQuery.exec();
	}

	return cachePath;
}

std::string AssetCacheDatabase::getMeshCachePath(std::string_view path)
{
	int64_t currentLastWriteTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(FileHelper::getAssetDirectoryPath() / path).time_since_epoch()).count();

	SQLite::Statement selectQuery(*_database,
		"SELECT guid, lastWriteTime FROM Mesh\n"
		"WHERE path=?;");

	selectQuery.bind(1, path.data(), path.size());

	xg::Guid guid;
	int64_t lastWriteTime;

	if (selectQuery.executeStep())
	{
		guid = columnToGuid(selectQuery.getColumn(0));
		lastWriteTime = selectQuery.getColumn(1).getInt64();
	}
	else
	{
		guid = xg::newGuid();
		lastWriteTime = currentLastWriteTime;

		SQLite::Statement insertQuery(*_database,
			"INSERT INTO Mesh\n"
			"VALUES(?, ?, ?);");

		insertQuery.bind(1, guid.bytes().data(), guid.bytes().size());
		insertQuery.bind(2, path.data(), path.size());
		insertQuery.bind(3, lastWriteTime);

		insertQuery.exec();
	}

	std::string cachePath = std::format("meshes/{}.c3dcache", guid.str());

	if (currentLastWriteTime != lastWriteTime)
	{
		std::filesystem::remove(FileHelper::getCacheAssetDirectoryPath() / cachePath);

		SQLite::Statement updateQuery(*_database,
			"UPDATE Mesh\n"
			"SET lastWriteTime=?\n"
			"WHERE guid=?;");

		updateQuery.bind(1, currentLastWriteTime);
		updateQuery.bind(2, guid.bytes().data(), guid.bytes().size());

		updateQuery.exec();
	}

	return cachePath;
}