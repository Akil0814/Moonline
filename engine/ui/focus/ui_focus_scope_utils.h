#pragma once

#include "ui_focus_scope.h"
#include "../input/ui_input_types.h"

#include <vector>

namespace elysia::ui
{
class UiControl;
class UiElement;

[[nodiscard]] bool is_navigation_action(UiAction action) noexcept;
[[nodiscard]] bool is_control_usable(const UiControl* control) noexcept;
void collect_live_controls(const UiElement& element,std::vector<const UiControl*>& out_controls);
void collect_live_scopes(const UiElement& element,std::vector<const UiFocusScope*>& out_scopes);
[[nodiscard]] bool contains_control(const std::vector<const UiControl*>& controls,const UiControl* control) noexcept;
[[nodiscard]] bool contains_scope(const std::vector<const UiFocusScope*>& scopes,const UiFocusScope* scope) noexcept;
}
