#include "ui_progress_bar.h"

#include "style/ui_theme.h"
#include "style/ui_style.h"

UiProgressBar::UiProgressBar(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiProgressBar::on_render(SDL_Renderer* renderer)
{
    refresh_theme_if_needed();
    _bar.render(renderer, rect());
}

void UiProgressBar::reset()
{
    UiElement::reset();
    _bar.reset();
}

void UiProgressBar::set_progress(float value)
{
    _bar.set_ratio(value);
}

float UiProgressBar::progress() const
{
    return _bar.ratio();
}

void UiProgressBar::set_bar_theme_role(UiBarThemeRole bar_theme_role)
{
    _bar_theme_role = bar_theme_role;
    mark_theme_dirty();
}

UiBarThemeRole UiProgressBar::bar_theme_role() const
{
    return _bar_theme_role;
}

Bar& UiProgressBar::bar()
{
    return _bar;
}

const Bar& UiProgressBar::bar() const
{
    return _bar;
}

void UiProgressBar::apply_theme(const UiTheme& theme)
{
    const BarStyle* style = _bar_theme_role == UiBarThemeRole::Progress
        ? &theme._progress_bar
        : &theme._default_bar;
    UiStyle::apply_bar(_bar, *style);
}
