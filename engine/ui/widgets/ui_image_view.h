#pragma once

#include "../base/ui_element.h"

#include <SDL.h>

#include <optional>
#include <string>

enum class ImageScaleMode
{
    Stretch,
    Fit,
    None
};

class UiImageView : public UiElement
{
public:
    explicit UiImageView(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_render(SDL_Renderer* renderer) override;
    void reset() override;

    void set_texture(SDL_Texture* texture);
    [[nodiscard]] SDL_Texture* texture() const;

    void set_texture_key(const std::string& texture_key);
    [[nodiscard]] const std::string& texture_key() const;

    void set_source_rect(const SDL_Rect& source_rect);
    void clear_source_rect();
    [[nodiscard]] bool has_source_rect() const;

    void set_tint_color(SDL_Color tint_color);
    [[nodiscard]] SDL_Color tint_color() const;

    void set_alpha(Uint8 alpha);
    [[nodiscard]] Uint8 alpha() const;

    void set_scale_mode(ImageScaleMode scale_mode);
    [[nodiscard]] ImageScaleMode scale_mode() const;

    void set_auto_size(bool auto_size);
    [[nodiscard]] bool auto_sizes() const;

private:
    [[nodiscard]] SDL_Texture* resolve_texture() const;
    [[nodiscard]] SDL_Rect destination_rect(int texture_width, int texture_height) const;
    void apply_theme(const UiTheme& theme) override;

private:
    SDL_Texture* _texture = nullptr;
    std::string _texture_key;
    std::optional<SDL_Rect> _source_rect;

    SDL_Color _tint_color{ 255, 255, 255, 255 };
    Uint8 _alpha = 255;

    ImageScaleMode _scale_mode = ImageScaleMode::Stretch;
    bool _auto_size = false;
};
