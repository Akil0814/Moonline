#pragma once

class UiSettings
{
public:
    static void set_animations_enabled(bool enabled)
    {
        _animations_enabled = enabled;
    }

    [[nodiscard]] static bool are_animations_enabled()
    {
        return _animations_enabled;
    }

private:
    inline static bool _animations_enabled = true;
};
