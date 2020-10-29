#pragma once

#include <mutex>

class Logger
{
public:
	enum class LogLevel
	{
		None = 0,
		Error = 1,
		Warning = 2,
		Full = 3
	};
	
	static void Init();
	
	static void Error(const char* message, const std::string& context = "Main");
	static void Error(const std::string& message, const std::string& context = "Main");
	static void Warning(const char* message, const std::string& context = "Main");
	static void Warning(const std::string& message, const std::string& context = "Main");
	static void Info(const char* message, const std::string& context = "Main");
	static void Info(const std::string& message, const std::string& context = "Main");
	
	static void SetLogLevel(LogLevel logLevel);

private:
	static Logger::LogLevel _logLevel;
	static std::mutex _mtx;
	static void* _consoleHandle;
	static unsigned short _defaultConsoleAttributes;
	static std::chrono::time_point<std::chrono::high_resolution_clock> _programStartTime;
	static void Print(const char* message, const std::string& context, const std::string& prefix, unsigned short color);
	
	static void SetColor(unsigned short color);
	static void ResetColor();
};
