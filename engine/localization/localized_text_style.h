#pragma once

#include "../core/render/color.h"

namespace elysia::localization
{
struct LocalizedTextStyle
{
	int point_size = 0;
	elysia::core::Color color{};
	int wrap_width = 0;
};

}
