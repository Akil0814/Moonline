#pragma once

#include "../core/ui_text_align.h"

namespace elysia::ui
{
// Semantic text roles map widget intent onto the finite set of preloaded font sizes.
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

// Rendering inputs resolved from a role; colors remain part of the widget's visual style.
struct UiResolvedTextStyle
{
    int point_size = 24;
    bool wrap_allowed = false;
    TextHorizontalAlign horizontal_align_default = TextHorizontalAlign::Left;
};

[[nodiscard]] inline UiResolvedTextStyle resolve_ui_typography(UiTypographyRole role) noexcept
{
    switch (role)
    {
    case UiTypographyRole::LabelMuted:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Title:
        return UiResolvedTextStyle{
            .point_size = 70,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Subtitle:
        return UiResolvedTextStyle{
            .point_size = 50,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Button:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::ButtonCompact:
        return UiResolvedTextStyle{
            .point_size = 20,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::Input:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::InputPlaceholder:
        return UiResolvedTextStyle{
            .point_size = 20,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Number:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogTitle:
        return UiResolvedTextStyle{
            .point_size = 60,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogBody:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = true,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::DialogAction:
        return UiResolvedTextStyle{
            .point_size = 20,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::SliderValue:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Center
        };
    case UiTypographyRole::CheckboxLabel:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::RadioLabel:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    case UiTypographyRole::Label:
    default:
        return UiResolvedTextStyle{
            .point_size = 30,
            .wrap_allowed = false,
            .horizontal_align_default = TextHorizontalAlign::Left
        };
    }
}
}
