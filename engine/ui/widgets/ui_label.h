#pragma once

#include <cstdint>
#include <string>

#include "../core/ui_element.h"


namespace elysia::ui
{
	class UiLabel : public UiElement
	{
		UiLabel(elysia::core::Vector2 position, elysia::core::Vector2 size, std::string text_key);
		UiLabel(elysia::core::Rect label_rect,std::string text_key);
		~UiLabel();

	};


}
