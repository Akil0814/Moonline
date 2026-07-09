#pragma once

#include "../input/ui_input_types.h"

#include <vector>

namespace elysia::core
{
struct UiRenderCommand;
}

namespace elysia::ui
{
class UiElement;

// A non-modal, window-rendered popup surface anchored by an ordinary UI control.
class UiTransientPopup
{
public:
    virtual ~UiTransientPopup() = default;

    [[nodiscard]] virtual UiElement& transient_popup_owner() noexcept = 0;
    [[nodiscard]] virtual const UiElement& transient_popup_owner() const noexcept = 0;
    [[nodiscard]] virtual bool is_transient_popup_open() const noexcept = 0;
    [[nodiscard]] virtual bool contains_transient_popup_point(int mouse_x,int mouse_y) const noexcept = 0;
    virtual void close_transient_popup() noexcept = 0;
    virtual bool on_transient_popup_input_event(const UiInputEvent& event) = 0;
    virtual void submit_transient_popup_render_commands(
        std::vector<elysia::core::UiRenderCommand>& out_commands) const = 0;
};
}
