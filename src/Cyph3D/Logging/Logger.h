#pragma once

#include <mutex>
#include "Cyph3D/Logging/LogColorFlags.h"
#include "Cyph3D/Logging/Impl/ILoggerColor.h"

class Logger
{
public:
	enum class LogLevel
	{
		NONE = 0,
		ERROR = 1,
		WARNING = 2,
		FULL = 3
	};
	
	static void error(std::string_view message, std::string_view context = "Main");
	static void warning(std::string_view message, std::string_view context = "Main");
	static void info(std::string_view message, std::string_view context = "Main");
	
	static void setLogLevel(LogLevel logLevel);

private:
	static Logger::LogLevel _logLevel;
	static std::mutex _mtx;
	static std::unique_ptr<ILoggerColor> _loggerColor;
	
	static void print(std::string_view message, std::string_view context, std::string_view prefix, LogColorFlags color);
};