#include "Engine.h"
#include "Exception/OpenGLException.h"
#include <windows.h>
#include "Logger.h"

int main()
{
	try
	{
		Logger::Init();
		Engine::init();
		Engine::run();
		Engine::shutdown();
	}
	catch (const OpenGLException& e)
	{
		Logger::Error(e.what(), "OPGL");
		if (IsDebuggerPresent())
			__debugbreak();
		Engine::shutdown();
		system("pause");
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		Logger::Error(e.what());
		if (IsDebuggerPresent())
			__debugbreak();
		Engine::shutdown();
		system("pause");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}