#include "Engine.h"
#include "Exception/OpenGLException.h"
#include "Logging/Logger.h"

int main(int argc, char** argv)
{
	try
	{
		Engine::init();
		Engine::run();
		Engine::shutdown();
	}
	catch (const OpenGLException& e)
	{
		Logger::error(e.what(), "OPGL");
		Engine::shutdown();
		system("pause");
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		Logger::error(e.what());
		Engine::shutdown();
		system("pause");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}