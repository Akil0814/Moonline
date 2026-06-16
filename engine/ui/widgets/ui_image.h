#pragma once

#include <cstdint>

#include "../core/ui_element.h"

struct SDL_Texture;

struct UiImageCenterTag
{
    explicit constexpr UiImageCenterTag() noexcept = default;
};

inline constexpr UiImageCenterTag k_ui_image_centered{};

class UiImage : public UiElement
{
public:
    UiImage(SDL_Texture* texture, Vector2 pos, Vector2 size, int order = 0);
    UiImage(SDL_Texture* texture, Rect rect, int order = 0);
    UiImage(
        SDL_Texture* texture,
        Vector2 center,
        Vector2 image_size,
        UiImageCenterTag,
        int order = 0
    );
    UiImage(
        SDL_Texture* texture,
        Vector2 center,
        Vector2 source_size,
        Vector2 render_size,
        UiImageCenterTag,
        int order = 0
    );

    void set_texture(SDL_Texture* texture);
    [[nodiscard]] SDL_Texture* texture() const;

    void set_source_rect(const Rect& rect);
    void clear_source_rect();

    void set_alpha(std::uint8_t alpha);
    [[nodiscard]] std::uint8_t alpha() const;

    void submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const override;

private:
    SDL_Texture* _texture = nullptr;

    bool _has_source_rect = false;
    Rect _source_rect{};

    std::uint8_t _alpha = 255;
};
