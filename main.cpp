#define SDL_MAIN_HANDLED
#include "application/application.h"

int main(int argc, char** argv)
{
	if (!Application::instance()->init(argc, argv))
		return -1;

	return Application::instance()->run(argc,argv);
}