#pragma once

namespace elysia::ui
{
// Theme roles are semantic lookup keys. They do not own colors and remain stable across built-in themes.
enum class UiPanelThemeRole
{
    Default,
    Screen,
    Dialog,
    List
};

enum class UiLabelThemeRole
{
    Default,
    Title,
    Subtitle,
    Muted
};

enum class UiTextBlockThemeRole
{
    Default,
    Muted
};

enum class UiButtonThemeRole
{
    Default,
    Primary,
    Danger
};

enum class UiBarThemeRole
{
    Default,
    Progress
};

enum class UiDialogThemeRole
{
    Default
};

enum class UiDropdownThemeRole
{
    Default
};

}
