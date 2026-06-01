#pragma once

#include "ui_element.h"
#include "ui_focusable.h"

class UiControl : public UiElement, public UiFocusable
{
public:
    explicit UiControl(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void reset() override;

    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;

    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;

    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

protected:
    bool _enabled = true;
    bool _is_focused = false;
};
