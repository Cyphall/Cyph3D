#pragma once

#include <glm/glm.hpp>
#include <fstream>
#include <mutex>

class ILoggerColor;

class Logger
{
public:
	enum class LogLevel
	{
		NONE = 0,
		ERROR = 1,
		WARNING = 2,
		INFO = 3,
		DEBUG = 4
	};

	static void error(std::string_view message, std::string_view context = "Main");
	static void warning(std::string_view message, std::string_view context = "Main");
	static void info(std::string_view message, std::string_view context = "Main");
	static void debug(std::string_view message, std::string_view context = "Main");

	static void setLogLevel(LogLevel logLevel);

private:
	static Logger::LogLevel _logLevel;
	static std::mutex _mtx;
	static std::unique_ptr<ILoggerColor> _loggerColor;
	static std::ofstream _logFile;

	static void print(std::string_view message, std::string_view context, std::string_view prefix, glm::u8vec3 prefixColor);
};