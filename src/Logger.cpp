#include "Logger.h"
#include <windows.h>
#include <chrono>
#include <iostream>
#include <iomanip>

Logger::LogLevel Logger::_logLevel = LogLevel::Full;
std::mutex Logger::_mtx;
std::chrono::time_point<std::chrono::high_resolution_clock> Logger::_programStartTime = std::chrono::high_resolution_clock::now();
void* Logger::_consoleHandle;
unsigned short Logger::_defaultConsoleAttributes;

void Logger::Print(const char* message, const std::string& context, const std::string& prefix, unsigned short color)
{
	_mtx.lock();
	auto diff = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - _programStartTime);
	
	std::cout << std::fixed << std::setprecision(4) << diff.count() << ' ';
	
	if (!context.empty())
	{
		std::cout << '[' << context << "] ";
	}
	
	SetColor(color);
	std::cout << prefix;
	ResetColor();
	
	std::cout << " > " << message << std::endl;
	_mtx.unlock();
}

void Logger::SetColor(unsigned short color)
{
	unsigned short attributes = _defaultConsoleAttributes & 0b1111111111110000;
	SetConsoleTextAttribute(_consoleHandle, attributes | color);
}

void Logger::ResetColor()
{
	SetConsoleTextAttribute(_consoleHandle, _defaultConsoleAttributes);
}

void Logger::Error(const char* message, const std::string& context)
{
	if (_logLevel < LogLevel::Error) return;
	
	Print(message, context, "ERROR", FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void Logger::Error(const std::string& message, const std::string& context)
{
	Error(message.c_str(), context);
}

void Logger::Warning(const char* message, const std::string& context)
{
	if (_logLevel < LogLevel::Warning) return;
	
	Print(message, context, "WARN", FOREGROUND_RED | FOREGROUND_GREEN);
}

void Logger::Warning(const std::string& message, const std::string& context)
{
	Warning(message.c_str(), context);
}

void Logger::Info(const char* message, const std::string& context)
{
	if (_logLevel < LogLevel::Full) return;
	
	Print(message, context, "INFO", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void Logger::Info(const std::string& message, const std::string& context)
{
	Info(message.c_str(), context);
}

void Logger::SetLogLevel(LogLevel logLevel)
{
	_logLevel = logLevel;
}

void Logger::Init()
{
	_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_consoleHandle, &info);
	_defaultConsoleAttributes = info.wAttributes;
}
