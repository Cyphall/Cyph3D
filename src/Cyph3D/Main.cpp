#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Logger.h"

int main(int argc, char** argv)
{
	try
	{
		Engine::init();
		Engine::run();
		Engine::shutdown();
	}
	catch (const std::exception& e)
	{
		Logger::error(e.what());
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}