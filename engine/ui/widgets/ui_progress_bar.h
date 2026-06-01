#pragma once
#include <SDL.h>
#include "../bar.h"
#include "../style/ui_theme_roles.h"
#include "../base/ui_element.h"

struct UiTheme;

class UiProgressBar : public UiElement
{
public:
    UiProgressBar(Vector2 position, Vector2 size, int order = 0);
    ~UiProgressBar() = default;

    void on_render(SDL_Renderer* renderer) override;
    void reset() override;
    void set_progress(float value);
    [[nodiscard]] float progress() const;
    void set_bar_theme_role(UiBarThemeRole bar_theme_role);
    [[nodiscard]] UiBarThemeRole bar_theme_role() const;

    Bar& bar();
    const Bar& bar() const;

private:
    void apply_theme(const UiTheme& theme) override;

private:
    Bar _bar;
    UiBarThemeRole _bar_theme_role = UiBarThemeRole::Progress;
};
