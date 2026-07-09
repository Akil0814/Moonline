#pragma once

#include "../core/ui_text_align.h"

#include <array>
#include <cstddef>
#include <cstdlib>

namespace elysia::ui
{
enum class UiTypographyRole
{
    Label,
    LabelMuted,
    Title,
    Subtitle,
    Button,
    ButtonCompact,
    Input,
    InputPlaceholder,
    Number,
    DialogTitle,
    DialogBody,
    DialogAction,
    SliderValue,
    CheckboxLabel,
    RadioLabel
};

struct UiResolvedTextStyle
{
    int point_size = 24;
    bool wrap_allowed = false;
    TextHorizontalAlign horizontal_align_default = TextHorizontalAlign::Left;
};

inline constexpr std::array<int,6> kLoadedUiFontPointSizes{ 12,16,18,22,24,26 };

[[nodiscard]] inline int resolve_loaded_ui_font_point_size(int requested_point_size) noexcept
{
    if (requested_point_size <= 0)
        return 24;

    int best_point_size = kLoadedUiFontPointSizes.front();
    int best_distance = std::abs(best_point_size - requested_point_size);
    for (std::size_t index = 1; index < kLoadedUiFontPointSizes.size(); ++index)
    {
        const int candidate = kLoadedUiFontPointSizes[index];
        const int candidate_distance = std::abs(candidate - requested_point_size);
        if (candidate_distance < best_distance)
        {
            best_point_size = candidate;
            best_distance = candidate_distance;
        }
    }

    return best_point_size;
}

[[nodiscard]] inline UiResolvedTextStyle resolve_ui_typography(UiTypographyRole role) noexcept
{
    switch (role)
    {
    case UiTypographyRole::LabelMuted:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Title:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(26),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Subtitle:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Button:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::ButtonCompact:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(16),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::Input:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::InputPlaceholder:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(18),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Number:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogTitle:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(26),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogBody:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(22),
            .wrap_allowed = true,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogAction:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(18),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::SliderValue:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(22),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::CheckboxLabel:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::RadioLabel:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Label:
    default:
        return UiResolvedTextStyle{
            .point_size = resolve_loaded_ui_font_point_size(24),
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    }
}
}
