#pragma once

#include <SDL.h>

struct LocalizedTextStyle
{
	int point_size = 0;
	SDL_Color color{ 255, 255, 255, 255 };
	int wrap_width = 0;
};
