#pragma once

#include <SDL.h>

enum class BarFillDirection
{
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

class Bar
{
public:
    Bar() = default;
    ~Bar() = default;

    void reset();

    void set_range(float min_value, float max_value);
    void set_value(float value);
    void set_ratio(float ratio);

    [[nodiscard]] float min_value() const;
    [[nodiscard]] float max_value() const;
    [[nodiscard]] float value() const;
    [[nodiscard]] float ratio() const;

    void set_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color background_color() const;

    void set_fill_color(SDL_Color color);
    [[nodiscard]] SDL_Color fill_color() const;

    void set_border_color(SDL_Color color);
    [[nodiscard]] SDL_Color border_color() const;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const;

    void set_fill_direction(BarFillDirection direction);
    [[nodiscard]] BarFillDirection fill_direction() const;

    void set_padding(int padding);
    [[nodiscard]] int padding() const;

    void render(SDL_Renderer* renderer, const SDL_Rect& rect) const;

private:
    [[nodiscard]] SDL_Rect content_rect(const SDL_Rect& rect) const;
    [[nodiscard]] SDL_Rect fill_rect(const SDL_Rect& rect) const;

private:
    float _min_value = 0.0f;
    float _max_value = 1.0f;
    float _value = 0.0f;

    SDL_Color _background_color{ 0, 43, 100, 255 };
    SDL_Color _fill_color{ 245, 255, 255, 255 };
    SDL_Color _border_color{ 0, 0, 0, 255 };

    BarFillDirection _fill_direction = BarFillDirection::LeftToRight;
    bool _draw_border = false;
    int _padding = 0;
};
