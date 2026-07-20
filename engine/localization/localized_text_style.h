#pragma once

#include "../core/render/color.h"
#include "../ui/text/ui_typography.h"

namespace elysia::localization
{
struct LocalizedTextStyle
{
	elysia::typography::UiTypographyRole typography_role =
		elysia::typography::UiTypographyRole::Label;
	elysia::core::Color color{};
	int wrap_width = 0;
};

}
