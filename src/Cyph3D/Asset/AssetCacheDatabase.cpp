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
		"CREATE TABLE IF NOT EXISTS Image"
		"("
		"	guid BINARY(16) NOT NULL PRIMARY KEY,"
		"	path TEXT NOT NULL,"
		"	lastWriteTime BIGINT NOT NULL,"
		"	format INT NOT NULL,"
		"	UNIQUE(path, lastWriteTime, format)"
		") WITHOUT ROWID;");

	_database->exec(
		"CREATE TABLE IF NOT EXISTS Mesh"
		"("
		"	guid BINARY(16) NOT NULL PRIMARY KEY,"
		"	path TEXT NOT NULL,"
		"	lastWriteTime BIGINT NOT NULL,"
		"	UNIQUE(path, lastWriteTime)"
		") WITHOUT ROWID;");
}

AssetCacheDatabase::~AssetCacheDatabase()
{}

xg::Guid AssetCacheDatabase::getImageCacheGuid(std::string_view path, const GLenum& format)
{
	int64_t lastWriteTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(FileHelper::getAssetDirectoryPath() / path).time_since_epoch()).count();
	
	SQLite::Statement selectQuery(*_database,
		"SELECT guid FROM Image\n"
		"WHERE path=? AND lastWriteTime=? AND format=?;");

	selectQuery.bind(1, path.data(), path.size());
	selectQuery.bind(2, lastWriteTime);
	selectQuery.bind(3, static_cast<uint32_t>(format));

	xg::Guid guid;
	
	if (selectQuery.executeStep())
	{
		guid = columnToGuid(selectQuery.getColumn(0));
	}
	else
	{
		guid = xg::newGuid();

		SQLite::Statement insertQuery(*_database,
			"INSERT INTO Image VALUES(?, ?, ?, ?);");

		insertQuery.bind(1, guid.bytes().data(), guid.bytes().size());
		insertQuery.bind(2, path.data(), path.size());
		insertQuery.bind(3, lastWriteTime);
		insertQuery.bind(4, static_cast<uint32_t>(format));

		insertQuery.exec();
	}

	return guid;
}

xg::Guid AssetCacheDatabase::getMeshCacheGuid(std::string_view path)
{
	int64_t lastWriteTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(FileHelper::getAssetDirectoryPath() / path).time_since_epoch()).count();

	SQLite::Statement selectQuery(*_database,
		"SELECT guid FROM Mesh\n"
		"WHERE path=? AND lastWriteTime=?;");

	selectQuery.bind(1, path.data(), path.size());
	selectQuery.bind(2, lastWriteTime);

	xg::Guid guid;

	if (selectQuery.executeStep())
	{
		guid = columnToGuid(selectQuery.getColumn(0));
	}
	else
	{
		guid = xg::newGuid();

		SQLite::Statement insertQuery(*_database,
			"INSERT INTO Mesh VALUES(?, ?, ?);");

		insertQuery.bind(1, guid.bytes().data(), guid.bytes().size());
		insertQuery.bind(2, path.data(), path.size());
		insertQuery.bind(3, lastWriteTime);

		insertQuery.exec();
	}

	return guid;
}