#pragma once

#include <SDL.h>

#include "../geometry/rect.h"
#include "../geometry/vector2.h"

enum class SpriteFlip
{
    None,
    Horizontal,
    Vertical,
    Both
};

enum class UiRenderCommandType
{
    Texture,
    FillRect,
    DrawRect,
    DrawLine
};

struct RenderCommand
{
    SDL_Texture* texture = nullptr;

    // Destination rectangle in world space.
    // This shuld be transformed by the camera before rendering.
    Rect command_rect{};

    Uint8 alpha = 255;

    // Source rectangle in texture space.
    // Defines which part of the texture or sprite sheet should be drawn.
    bool use_src_rect = false;
    Rect src_rect{};

    // Clockwise rotation angle in degrees.
    double rotation_degrees = 0.0;

    // Normalized rotation origin inside _world_rect.
    // (0.5, 0.5) -> center, (0, 0) -> top-left
    Vector2 rotation_origin = Vector2(0.5f, 0.5f);

    SpriteFlip flip = SpriteFlip::None;
};

struct UiRenderCommand
{
    UiRenderCommandType type = UiRenderCommandType::Texture;

    SDL_Texture* texture = nullptr;

    // Destination rectangle in screen/UI space.
    // This is not affected by the world camera.
    Rect screen_rect{};

    SDL_Color color{ 255, 255, 255, 255 };

    Vector2 line_start{};
    Vector2 line_end{};

    Uint8 alpha = 255;

    // Source rectangle in texture space.
    // Defines which part of the texture or sprite sheet should be drawn.
    // If false, the full texture is drawn.
    bool use_src_rect = false;
    Rect src_rect{};

    // Clockwise rotation angle in degrees.
    double rotation_degrees = 0.0;

    // Normalized rotation origin inside _screen_rect.
    // (0.5, 0.5) -> center, (0, 0) -> top-left
    Vector2 rotation_origin = Vector2(0.5f, 0.5f);

    SpriteFlip flip = SpriteFlip::None;
};

[[nodiscard]] inline UiRenderCommand make_ui_texture_command(
    SDL_Texture* texture,
    const Rect& screen_rect,
    Uint8 alpha = 255
) noexcept
{
    UiRenderCommand command;
    command.type = UiRenderCommandType::Texture;
    command.texture = texture;
    command.screen_rect = screen_rect;
    command.alpha = alpha;
    return command;
}

[[nodiscard]] inline UiRenderCommand make_ui_fill_rect_command(
    const Rect& screen_rect,
    SDL_Color color
) noexcept
{
    UiRenderCommand command;
    command.type = UiRenderCommandType::FillRect;
    command.screen_rect = screen_rect;
    command.color = color;
    return command;
}

[[nodiscard]] inline UiRenderCommand make_ui_draw_rect_command(
    const Rect& screen_rect,
    SDL_Color color
) noexcept
{
    UiRenderCommand command;
    command.type = UiRenderCommandType::DrawRect;
    command.screen_rect = screen_rect;
    command.color = color;
    return command;
}

[[nodiscard]] inline UiRenderCommand make_ui_draw_line_command(
    const Vector2& line_start,
    const Vector2& line_end,
    SDL_Color color
) noexcept
{
    UiRenderCommand command;
    command.type = UiRenderCommandType::DrawLine;
    command.line_start = line_start;
    command.line_end = line_end;
    command.color = color;
    return command;
}
