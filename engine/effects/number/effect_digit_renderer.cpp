#include "effect_digit_renderer.h"

#include <SDL.h>

#include <algorithm>
#include <vector>

namespace elysia::effects
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

float resolve_scale(const GlyphLayout& glyph, const EffectDigitRenderRequest& request)
{
    if (glyph.texture_height <= 0)
    {
        return 0.0f;
    }

    float scale = std::max(0.0f, request.uniform_scale);
    if (request.glyph_size.has_value() && request.glyph_size->y > 0.0f)
    {
        scale = request.glyph_size->y / static_cast<float>(glyph.texture_height);
    }
    else if (request.target_height.has_value() && *request.target_height > 0.0f)
    {
        scale = *request.target_height / static_cast<float>(glyph.texture_height);
    }
    else if (request.world_rect.has_value() && request.world_rect->height() > 0.0f)
    {
        scale = request.world_rect->height() / static_cast<float>(glyph.texture_height);
    }

    return std::max(0.0f, scale);
}
}

EffectDigitRenderer::EffectDigitRenderer(elysia::number::DigitCache* digit_cache) noexcept
    : _digit_cache(digit_cache)
{
}

void EffectDigitRenderer::set_digit_cache(elysia::number::DigitCache* digit_cache) noexcept
{
    _digit_cache = digit_cache;
}

elysia::number::DigitCache* EffectDigitRenderer::digit_cache() const noexcept
{
    return _digit_cache;
}

void EffectDigitRenderer::append_render_commands(
    const EffectDigitRenderRequest& request,
    std::vector<elysia::core::RenderCommand>& out_commands
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

        const float scale = resolve_scale(glyph, request);
        if (request.glyph_size.has_value() && request.glyph_size->x > 0.0f)
        {
            glyph.render_width = request.glyph_size->x;
            glyph.render_height = request.glyph_size->y;
        }
        else
        {
            glyph.render_width = static_cast<float>(texture_width) * scale;
            glyph.render_height = static_cast<float>(texture_height) * scale;
        }

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
    if (request.world_rect.has_value())
    {
        const elysia::core::Rect& target = *request.world_rect;
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
    else if (request.world_position.has_value())
    {
        const elysia::core::Vector2 anchor = *request.world_position;
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
        if (request.world_rect.has_value())
        {
            render_y = request.world_rect->center().y - glyph.render_height * 0.5f;
        }

        elysia::core::RenderCommand command;
        command.texture = glyph.texture;
        command.command_rect = elysia::core::Rect(
            cursor_x,
            render_y,
            glyph.render_width,
            glyph.render_height
        );
        command.alpha = request.alpha;
        command.rotation_degrees = request.rotation_degrees;
        command.rotation_origin = request.rotation_origin;
        command.flip = request.flip;
        out_commands.push_back(command);

        cursor_x += glyph.advance;
        if (index + 1 < glyphs.size())
        {
            cursor_x += std::max(0.0f, request.spacing);
        }
    }
}

}
