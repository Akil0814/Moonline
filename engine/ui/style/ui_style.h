#pragma once

#include <optional>

namespace elysia::ui
{
template<class Style>
class UiStyleState
{
public:
    // Resets both theme-derived and manual state. Callers typically use this from reset()
    // so controls fall back to pure theme ownership until an explicit override is applied.
    void reset(const Style& theme_style) noexcept
    {
        _theme_style = theme_style;
        _style_override.reset();
    }

    // Updates the theme-owned style without disturbing any active manual override.
    void set_theme_style(const Style& style) noexcept
    {
        _theme_style = style;
    }

    [[nodiscard]] const Style& theme_style() const noexcept
    {
        return _theme_style;
    }

    // A style override always wins over later theme refreshes until cleared explicitly.
    void set_style_override(const Style& style) noexcept
    {
        _style_override = style;
    }

    [[nodiscard]] bool has_style_override() const noexcept
    {
        return _style_override.has_value();
    }

    void clear_style_override() noexcept
    {
        _style_override.reset();
    }

    // Lazily materializes a manual override from the current effective style so callers can
    // tweak one field without losing the rest of the resolved theme-driven state.
    [[nodiscard]] Style& ensure_style_override() noexcept
    {
        if (!_style_override.has_value())
            _style_override = effective_style();
        return *_style_override;
    }

    [[nodiscard]] const std::optional<Style>& style_override() const noexcept
    {
        return _style_override;
    }

    // Rendering and layout should always read through effective_style() so manual overrides
    // transparently shadow the current theme style.
    [[nodiscard]] const Style& effective_style() const noexcept
    {
        return _style_override.has_value() ? *_style_override : _theme_style;
    }

private:
    Style _theme_style{};
    std::optional<Style> _style_override;
};
}
