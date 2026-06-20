#pragma once

#include "../core/render/color.h"

struct LocalizedTextStyle
{
	int point_size = 0;
	Color color{};
	int wrap_width = 0;
};
