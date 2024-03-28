#pragma once

#include <format>
#include <fstream>
#include <glm/glm.hpp>
#include <mutex>

class Logger
{
public:
	enum class LogLevel
	{
		eDisabled = 0,
		eError = 1,
		eWarning = 2,
		eInfo = 3,
		eDebug = 4
	};

	static void error(std::string_view message);
	static void warning(std::string_view message);
	static void info(std::string_view message);
	static void debug(std::string_view message);

	template<typename... TArgs>
	static void error(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::eError)
			return;

		error(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void warning(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::eWarning)
			return;

		warning(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void info(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::eInfo)
			return;

		info(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	template<typename... TArgs>
	static void debug(std::format_string<TArgs...> fmt, TArgs&&... args)
	{
		if (_logLevel < LogLevel::eDebug)
			return;

		debug(std::format(std::forward<std::format_string<TArgs...>>(fmt), std::forward<TArgs>(args)...));
	}

	static void init(LogLevel logLevel);

private:
	static Logger::LogLevel _logLevel;
	static std::mutex _mtx;
	static std::ofstream _logFile;

	static void print(std::string_view message, std::string_view prefix, std::string_view prefixColor);
};