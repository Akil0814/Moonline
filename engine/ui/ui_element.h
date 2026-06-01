#pragma once

#include "../core/game_object.h"

struct UiTheme;

class UiElement : public GameObject
{
public:
    explicit UiElement(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);
    ~UiElement() override;

    UiElement(const UiElement&) = delete;
    UiElement& operator=(const UiElement&) = delete;
    UiElement(UiElement&&) = delete;
    UiElement& operator=(UiElement&&) = delete;

    void reset() override;

    void set_use_theme(bool use_theme);
    [[nodiscard]] bool uses_theme() const;

    void mark_theme_dirty();
    void refresh_theme_if_needed();

protected:
    virtual void apply_theme(const UiTheme& theme) = 0;

private:
    bool _use_theme = true;
    bool _theme_dirty = true;
};
