#pragma once

#include "../../../core/render/colors.h"
#include "../../core/ui_element.h"

#include <string>

struct SDL_Texture;

namespace elysia::ui
{
enum class TextHorizontalAlign
{
    Left,
    Center,
    Right
};

enum class TextVerticalAlign
{
    Top,
    Center,
    Bottom
};

class UiLabel : public UiElement
{
public:
    explicit UiLabel(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0,std::string text_key = {}) noexcept;
    UiLabel(
        const elysia::core::Vector2& position,
        const elysia::core::Vector2& size,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiLabel(
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& size,
        UiFromCenterTag,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    ~UiLabel() override = default;

    void reset() noexcept override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_text_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_background_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color background_color() const noexcept;

    void set_draw_background(bool draw_background);
    [[nodiscard]] bool draws_background() const noexcept;

    void set_horizontal_align(TextHorizontalAlign align);
    [[nodiscard]] TextHorizontalAlign horizontal_align() const noexcept;

    void set_vertical_align(TextVerticalAlign align);
    [[nodiscard]] TextVerticalAlign vertical_align() const noexcept;

    void set_text_point_size(int point_size);
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

private:
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;

private:
    std::string _text_key;
    elysia::core::Color _text_color = elysia::core::colors::white;
    elysia::core::Color _background_color = elysia::core::colors::transparent;
    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    TextVerticalAlign _vertical_align = TextVerticalAlign::Top;
    int _text_point_size = 24;
    int _padding = 0;
    bool _draw_background = false;
};
}