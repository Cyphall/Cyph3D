#include "Cyph3D/Engine.h"

#include <spdlog/spdlog.h>

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
		spdlog::error(e.what());
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}