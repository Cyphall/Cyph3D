#pragma once

#include <format>
#include <fstream>
#include <glm/glm.hpp>
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

	static void error(std::string_view message);
	static void warning(std::string_view message);
	static void info(std::string_view message);
	static void debug(std::string_view message);

	template<typename... TArgs>
	static void error(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::ERROR)
			return;

		error(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void warning(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::WARNING)
			return;

		warning(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void info(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::INFO)
			return;

		info(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void debug(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::DEBUG)
			return;

		debug(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	static void setLogLevel(LogLevel logLevel);

private:
	static Logger::LogLevel _logLevel;
	static std::mutex _mtx;
	static std::unique_ptr<ILoggerColor> _loggerColor;
	static std::ofstream _logFile;

	static void print(std::string_view message, std::string_view prefix, glm::u8vec3 prefixColor);
};