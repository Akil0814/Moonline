#pragma once

#include "../input/ui_input_types.h"

#include <optional>

namespace elysia::ui
{
class UiControl;
class UiElement;
class UiFocusScope;

struct UiFocusNeighbors
{
    std::optional<UiControl*> up;
    std::optional<UiControl*> down;
    std::optional<UiControl*> left;
    std::optional<UiControl*> right;
};

struct UiFocusScopeNeighbors
{
    UiFocusScope* up = nullptr;
    UiFocusScope* down = nullptr;
    UiFocusScope* left = nullptr;
    UiFocusScope* right = nullptr;
};

class UiFocusScope
{
public:
    virtual ~UiFocusScope() = default;

    [[nodiscard]] virtual UiElement& focus_scope_element() noexcept = 0;
    [[nodiscard]] virtual const UiElement& focus_scope_element() const noexcept = 0;
    virtual void set_scope_focused(bool focused) noexcept = 0;
    [[nodiscard]] virtual bool is_scope_focused() const noexcept = 0;
    [[nodiscard]] virtual bool has_focusable_target() const noexcept = 0;
    virtual bool focus_first_available() = 0;
    [[nodiscard]] virtual UiControl* focused_target() const noexcept = 0;
    [[nodiscard]] virtual bool can_navigate(UiAction action) const noexcept = 0;
    [[nodiscard]] virtual bool contains_focus_point(int mouse_x,int mouse_y) const noexcept = 0;
};
}
