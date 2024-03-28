#include "Logger.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"

#include <iostream>
#include <Windows.h>

Logger::LogLevel Logger::_logLevel = LogLevel::eDisabled;
std::mutex Logger::_mtx;
std::ofstream Logger::_logFile = FileHelper::openFileForWriting("Cyph3D.log");

static constexpr const char* ERROR_COLOR = "\x1b[38;2;255;51;102m";
static constexpr const char* WARNING_COLOR = "\x1b[38;2;255;204;85m";
static constexpr const char* INFO_COLOR = "\x1b[38;2;22;198;12m";
static constexpr const char* DEBUG_COLOR = "\x1b[38;2;160;160;160m";
static constexpr const char* DEFAULT_ATTRIBUTES = "\x1b[0m";

void Logger::print(std::string_view message, std::string_view prefix, std::string_view prefixColor)
{
	_mtx.lock();

	std::cout << DEFAULT_ATTRIBUTES;

	std::string time = std::format("{:.4f}", Engine::getTimer().time());

	std::cout << time << " ";
	_logFile << time << " ";

	std::cout << prefixColor;

	std::cout << prefix;
	_logFile << prefix;

	std::cout << DEFAULT_ATTRIBUTES;

	std::cout << " > " << message << std::endl;
	_logFile << " > " << message << std::endl;

	_mtx.unlock();
}

void Logger::error(std::string_view message)
{
	if (_logLevel < LogLevel::eError)
		return;

	print(message, "ERROR", ERROR_COLOR);
}

void Logger::warning(std::string_view message)
{
	if (_logLevel < LogLevel::eWarning)
		return;

	print(message, "WARNING", WARNING_COLOR);
}

void Logger::info(std::string_view message)
{
	if (_logLevel < LogLevel::eInfo)
		return;

	print(message, "INFO", INFO_COLOR);
}

void Logger::debug(std::string_view message)
{
	if (_logLevel < LogLevel::eDebug)
		return;

	print(message, "DEBUG", DEBUG_COLOR);
}

void Logger::init(LogLevel logLevel)
{
	_logLevel = logLevel;

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD mode;
	GetConsoleMode(consoleHandle, &mode);

	mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(consoleHandle, mode);
}