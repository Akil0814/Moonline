#define SDL_MAIN_HANDLED
#include "engine/application/application.h"
#include "gameplay/application/moonline_game_module.h"

#include <cstdlib>

int main(int argc, char** argv)
{
	moonline::application::MoonlineGameModule game_module;
	auto* application = elysia::application::Application::instance();
	if (!application->init(argc,argv,game_module))
		return EXIT_FAILURE;

	return application->run();
}
