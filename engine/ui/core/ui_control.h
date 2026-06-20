#pragma once

#include "ui_element.h"
#include "ui_focusable.h"

namespace elysia::ui
{
class UiControl : public UiElement, public UiFocusable
{
public:
    explicit UiControl(elysia::core::Vector2 position = elysia::core::Vector2::zero(), elysia::core::Vector2 size = elysia::core::Vector2::zero(), int order = 0);

    void reset() noexcept override;

    void set_enabled(bool enabled);
    [[nodiscard]] bool is_enabled() const;

    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;

protected:
    bool _enabled = true;
    bool _is_focused = false;
};

}
