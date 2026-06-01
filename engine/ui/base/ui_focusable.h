#pragma once

#include "../../core/game_object.h"
#include "../../input/input_types.h"

class UiFocusable
{
public:
    virtual ~UiFocusable() = default;

    virtual void set_focused(bool focused) = 0;
    [[nodiscard]] virtual bool is_focused() const = 0;

    virtual void set_enabled(bool enabled) = 0;
    [[nodiscard]] virtual bool is_enabled() const = 0;

    [[nodiscard]] virtual bool handle_focused_input_event(const InputEvent& event) = 0;

    [[nodiscard]] virtual GameObject* game_object() = 0;
    [[nodiscard]] virtual const GameObject* game_object() const = 0;
};
