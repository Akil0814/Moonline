#include "ui_digit_renderer.h"

#include <SDL.h>

#include <algorithm>
#include <vector>

namespace elysia::ui
{
namespace
{
struct GlyphLayout
{
    SDL_Texture* texture = nullptr;
    int texture_width = 0;
    int texture_height = 0;
    float render_width = 0.0f;
    float render_height = 0.0f;
    float advance = 0.0f;
};

float resolve_scale(
    const GlyphLayout& glyph,
    const UiDigitRenderRequest& request,
    const std::optional<elysia::core::Rect>& target_rect
)
{
    if (glyph.texture_height <= 0)
    {
        return 0.0f;
    }

    float scale = std::max(0.0f, request.uniform_scale);
    if (request.target_height.has_value() && *request.target_height > 0.0f)
    {
        scale = *request.target_height / static_cast<float>(glyph.texture_height);
    }
    else if (target_rect.has_value() && target_rect->height() > 0.0f)
    {
        scale = target_rect->height() / static_cast<float>(glyph.texture_height);
    }

    return std::max(0.0f, scale);
}
}

UiDigitRenderer::UiDigitRenderer(elysia::number::DigitCache* digit_cache) noexcept
    : _digit_cache(digit_cache)
{
}

void UiDigitRenderer::set_digit_cache(elysia::number::DigitCache* digit_cache) noexcept
{
    _digit_cache = digit_cache;
}

elysia::number::DigitCache* UiDigitRenderer::digit_cache() const noexcept
{
    return _digit_cache;
}

void UiDigitRenderer::append_render_commands(
    const UiDigitRenderRequest& request,
    std::vector<elysia::core::UiRenderCommand>& out_commands
) const
{
    if (!_digit_cache || request.text.empty())
    {
        return;
    }

    std::vector<GlyphLayout> glyphs;
    glyphs.reserve(request.text.size());

    float total_width = 0.0f;
    for (const char ch : request.text)
    {
        SDL_Texture* texture = _digit_cache->get_glyph(ch);
        if (!texture)
        {
            continue;
        }

        int texture_width = 0;
        int texture_height = 0;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height) != 0
            || texture_width <= 0
            || texture_height <= 0)
        {
            continue;
        }

        GlyphLayout glyph;
        glyph.texture = texture;
        glyph.texture_width = texture_width;
        glyph.texture_height = texture_height;

        const float scale = resolve_scale(glyph, request, request.target_rect);
        glyph.render_width = static_cast<float>(texture_width) * scale;
        glyph.render_height = static_cast<float>(texture_height) * scale;
        glyph.advance = request.fixed_glyph_advance.has_value()
            ? std::max(0.0f, *request.fixed_glyph_advance)
            : glyph.render_width;

        if (!glyphs.empty())
        {
            total_width += std::max(0.0f, request.spacing);
        }

        total_width += glyph.advance;
        glyphs.push_back(glyph);
    }

    if (glyphs.empty())
    {
        return;
    }

    float origin_x = 0.0f;
    float baseline_y = 0.0f;
    if (request.target_rect.has_value())
    {
        const elysia::core::Rect& target = *request.target_rect;
        switch (request.alignment)
        {
        case elysia::number::DigitAlignment::Center:
            origin_x = target.center().x - total_width * 0.5f;
            break;

        case elysia::number::DigitAlignment::Right:
            origin_x = target.right() - total_width;
            break;

        case elysia::number::DigitAlignment::Left:
        default:
            origin_x = target.x();
            break;
        }

        baseline_y = target.y();
    }
    else if (request.anchor_position.has_value())
    {
        const elysia::core::Vector2 anchor = *request.anchor_position;
        switch (request.alignment)
        {
        case elysia::number::DigitAlignment::Center:
            origin_x = anchor.x - total_width * 0.5f;
            break;

        case elysia::number::DigitAlignment::Right:
            origin_x = anchor.x - total_width;
            break;

        case elysia::number::DigitAlignment::Left:
        default:
            origin_x = anchor.x;
            break;
        }

        baseline_y = anchor.y;
    }
    else
    {
        return;
    }

    float cursor_x = origin_x;
    for (std::size_t index = 0; index < glyphs.size(); ++index)
    {
        const GlyphLayout& glyph = glyphs[index];
        float render_y = baseline_y;
        if (request.target_rect.has_value())
        {
            switch (request.vertical_align)
            {
            case TextVerticalAlign::Top:
                render_y = request.target_rect->y();
                break;
            case TextVerticalAlign::Bottom:
                render_y = request.target_rect->bottom() - glyph.render_height;
                break;
            case TextVerticalAlign::Center:
            default:
                render_y = request.target_rect->center().y - glyph.render_height * 0.5f;
                break;
            }
        }
        else if (request.anchor_position.has_value())
        {
            switch (request.vertical_align)
            {
            case TextVerticalAlign::Top:
                render_y = request.anchor_position->y;
                break;
            case TextVerticalAlign::Bottom:
                render_y = request.anchor_position->y - glyph.render_height;
                break;
            case TextVerticalAlign::Center:
            default:
                render_y = request.anchor_position->y - glyph.render_height * 0.5f;
                break;
            }
        }

        const elysia::core::Rect render_rect(
            cursor_x,
            render_y,
            glyph.render_width,
            glyph.render_height
        );
        out_commands.push_back(
            elysia::core::make_ui_texture_command(glyph.texture, render_rect, request.alpha)
        );

        cursor_x += glyph.advance;
        if (index + 1 < glyphs.size())
        {
            cursor_x += std::max(0.0f, request.spacing);
        }
    }
}

}
