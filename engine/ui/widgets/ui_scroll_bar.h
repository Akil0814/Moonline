#pragma once

#include "../containers/ui_scroll_panel.h"
#include "../ui_element.h"

#include <SDL.h>

enum class ScrollBarOrientation
{
    Vertical,
    Horizontal
};

class UiScrollBar : public UiElement
{
public:
    explicit UiScrollBar(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_target(UiScrollPanel* target);
    [[nodiscard]] UiScrollPanel* target() const;

    void set_orientation(ScrollBarOrientation orientation);
    [[nodiscard]] ScrollBarOrientation orientation() const;

    void set_auto_follow_target(bool auto_follow_target);
    [[nodiscard]] bool auto_follows_target() const;

    void set_target_margin(float margin);
    [[nodiscard]] float target_margin() const;

    void set_thickness(float thickness);
    [[nodiscard]] float thickness() const;

    void set_min_thumb_size(float min_thumb_size);
    [[nodiscard]] float min_thumb_size() const;

    void set_track_color(SDL_Color color);
    [[nodiscard]] SDL_Color track_color() const;

    void set_thumb_color(SDL_Color color);
    [[nodiscard]] SDL_Color thumb_color() const;

    void set_draw_track(bool draw_track);
    [[nodiscard]] bool draws_track() const;

    void set_auto_hide(bool auto_hide);
    [[nodiscard]] bool auto_hides() const;

private:
    void sync_with_target();
    [[nodiscard]] SDL_Rect track_rect() const;
    [[nodiscard]] SDL_Rect thumb_rect() const;
    [[nodiscard]] bool should_draw() const;
    [[nodiscard]] bool contains_point(const SDL_Rect& target_rect, int x, int y) const;
    void update_target_from_point(int x, int y);
    void apply_theme(const UiTheme& theme) override;

private:
    UiScrollPanel* _target = nullptr;

    SDL_Color _track_color{ 32, 40, 56, 180 };
    SDL_Color _thumb_color{ 126, 156, 196, 255 };

    float _target_margin = 6.0f;
    float _thickness = 8.0f;
    float _min_thumb_size = 24.0f;

    float _drag_offset = 0.0f;

    bool _auto_follow_target = true;
    bool _draw_track = true;
    bool _auto_hide = true;
    bool _is_dragging = false;
    bool _was_mouse_down = false;

    ScrollBarOrientation _orientation = ScrollBarOrientation::Vertical;
};
