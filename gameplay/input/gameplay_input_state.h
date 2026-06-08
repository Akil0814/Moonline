#pragma once

#include "gameplay_input_types.h"

#include <array>
#include <cstddef>

class GameplayInputState
{
public:
    void set(GameplayAction action, bool pressed, bool just_pressed, bool just_released)
    {
        if (!is_trackable_action(action))
        {
            return;
        }

        const std::size_t i = index(action);
        _pressed[i] = pressed;
        _just_pressed[i] = just_pressed;
        _just_released[i] = just_released;
    }

    [[nodiscard]] bool is_pressed(GameplayAction action) const
    {
        return get(_pressed, action);
    }

    [[nodiscard]] bool is_just_pressed(GameplayAction action) const
    {
        return get(_just_pressed, action);
    }

    [[nodiscard]] bool is_just_released(GameplayAction action) const
    {
        return get(_just_released, action);
    }

private:
    static constexpr bool is_trackable_action(GameplayAction action)
    {
        return action != GameplayAction::None && action != GameplayAction::Count;
    }

    static constexpr std::size_t index(GameplayAction action)
    {
        return static_cast<std::size_t>(action);
    }

    [[nodiscard]] bool get(
        const std::array<bool, static_cast<std::size_t>(GameplayAction::Count)>& values,
        GameplayAction action
    ) const
    {
        if (!is_trackable_action(action))
        {
            return false;
        }

        return values[index(action)];
    }

private:
    std::array<bool, static_cast<std::size_t>(GameplayAction::Count)> _pressed{};
    std::array<bool, static_cast<std::size_t>(GameplayAction::Count)> _just_pressed{};
    std::array<bool, static_cast<std::size_t>(GameplayAction::Count)> _just_released{};
};
