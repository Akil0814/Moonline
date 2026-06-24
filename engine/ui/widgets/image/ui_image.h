#pragma once

#include <cstdint>

#include "../../core/ui_element.h"

struct SDL_Texture;

namespace elysia::ui
{
class UiImage : public UiElement
{
public:
    UiImage(SDL_Texture* texture,elysia::core::Vector2 pos,elysia::core::Vector2 size,int order = 0);
    UiImage(SDL_Texture* texture,elysia::core::Rect rect,int order = 0);
    UiImage(SDL_Texture* texture,elysia::core::Vector2 center,elysia::core::Vector2 image_size,UiFromCenterTag,int order = 0);
    UiImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 source_size,
        elysia::core::Vector2 render_size,
        UiFromCenterTag,
        int order = 0
    );

    void set_texture(SDL_Texture* texture);
    [[nodiscard]] SDL_Texture* texture() const;

    void set_source_rect(const elysia::core::Rect& rect);
    void clear_source_rect();

    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

private:
    SDL_Texture* _texture = nullptr;
    bool _has_source_rect = false;
    elysia::core::Rect _source_rect{};
};
}