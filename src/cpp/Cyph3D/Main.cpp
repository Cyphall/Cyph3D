#include <Cyph3D/Engine.h>

#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
	try
	{
		c3d::Engine::init();
		c3d::Engine::run();
		c3d::Engine::shutdown();
	}
	catch (const std::exception& e)
	{
		spdlog::error(e.what());
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}