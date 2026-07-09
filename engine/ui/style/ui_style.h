#pragma once

#include <optional>

namespace elysia::ui
{
template<class Style>
class UiStyleState
{
public:
    void reset(const Style& theme_style) noexcept
    {
        _theme_style = theme_style;
        _style_override.reset();
    }

    void set_theme_style(const Style& style) noexcept
    {
        _theme_style = style;
    }

    [[nodiscard]] const Style& theme_style() const noexcept
    {
        return _theme_style;
    }

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

    [[nodiscard]] const Style& effective_style() const noexcept
    {
        return _style_override.has_value() ? *_style_override : _theme_style;
    }

private:
    Style _theme_style{};
    std::optional<Style> _style_override;
};
}
