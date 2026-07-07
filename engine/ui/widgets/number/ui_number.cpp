#include "ui_number.h"

#include "../../style/ui_style_defaults.h"
#include "../../../core/render/render_command.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

namespace elysia::ui
{
namespace
{
constexpr double kNegativeZeroTolerance = 1e-9;

struct GlyphLayout
{
    SDL_Texture* texture = nullptr;
    float render_width = 0.0f;
    float render_height = 0.0f;
    float advance = 0.0f;
};
}

UiNumber::UiNumber(const elysia::core::Rect& rect,int order) noexcept
    : UiElement(rect,order)
{
    reset();
}

UiNumber::UiNumber(
    const elysia::core::Vector2& position,
    const elysia::core::Vector2& size,
    int order
) noexcept
    : UiElement(position,size,order)
{
    reset();
}

UiNumber::UiNumber(
    const elysia::core::Vector2& center,
    const elysia::core::Vector2& size,
    UiFromCenterTag,
    int order
) noexcept
    : UiElement(center,size,from_center,order)
{
    reset();
}

void UiNumber::reset() noexcept
{
    UiElement::reset();
    _texture_provider.reset();
    _value = 0.0;
    _style = UiStyleDefaults::number();
    _horizontal_align = TextHorizontalAlign::Left;
    _vertical_align = TextVerticalAlign::Top;
    _text_point_size = 24;
    _padding = 0;
    _digit_spacing = 0.0f;
    _fixed_glyph_advance.reset();
    _target_height.reset();
    _decimal_places = 0;
    _trim_trailing_zeros = true;
    _keep_decimal_point = false;
    _suffix = UiNumberSuffix::None;
}

void UiNumber::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& number_rect = screen_rect();
    if (number_rect.is_empty())
        return;

    if (_style.draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(number_rect,apply_opacity(_style.background)));

    const std::string text = formatted_text();
    if (text.empty())
        return;

    const elysia::core::Rect available_rect = content_rect();
    if (available_rect.is_empty())
        return;

    const std::vector<elysia::number::NumberTextureGlyph> texture_set =
        _texture_provider.get_texture_set(text,_text_point_size,_style.text);
    if (texture_set.empty())
        return;

    std::vector<GlyphLayout> glyphs;
    glyphs.reserve(texture_set.size());

    float total_width = 0.0f;
    for (const elysia::number::NumberTextureGlyph& texture_glyph : texture_set)
    {
        if (!texture_glyph.texture || texture_glyph.texture_width <= 0 || texture_glyph.texture_height <= 0)
            continue;

        float scale = 0.0f;
        if (_target_height.has_value() && *_target_height > 0.0f)
            scale = *_target_height / static_cast<float>(texture_glyph.texture_height);
        else if (available_rect.height() > 0.0f)
            scale = available_rect.height() / static_cast<float>(texture_glyph.texture_height);

        scale = std::max(0.0f,scale);

        GlyphLayout glyph;
        glyph.texture = texture_glyph.texture;
        glyph.render_width = static_cast<float>(texture_glyph.texture_width) * scale;
        glyph.render_height = static_cast<float>(texture_glyph.texture_height) * scale;
        glyph.advance = _fixed_glyph_advance.has_value()
            ? std::max(0.0f,*_fixed_glyph_advance)
            : glyph.render_width;

        if (!glyphs.empty())
            total_width += _digit_spacing;

        total_width += glyph.advance;
        glyphs.push_back(glyph);
    }

    if (glyphs.empty())
        return;

    float origin_x = available_rect.x();
    switch (digit_alignment())
    {
    case elysia::number::DigitAlignment::Center:
        origin_x = available_rect.center().x - total_width * 0.5f;
        break;
    case elysia::number::DigitAlignment::Right:
        origin_x = available_rect.right() - total_width;
        break;
    case elysia::number::DigitAlignment::Left:
    default:
        origin_x = available_rect.x();
        break;
    }

    float cursor_x = origin_x;
    for (std::size_t index = 0; index < glyphs.size(); ++index)
    {
        const GlyphLayout& glyph = glyphs[index];

        float render_y = available_rect.y();
        switch (_vertical_align)
        {
        case TextVerticalAlign::Center:
            render_y = available_rect.center().y - glyph.render_height * 0.5f;
            break;
        case TextVerticalAlign::Bottom:
            render_y = available_rect.bottom() - glyph.render_height;
            break;
        case TextVerticalAlign::Top:
        default:
            render_y = available_rect.y();
            break;
        }

        elysia::core::UiRenderCommand command = elysia::core::make_ui_texture_command(
            glyph.texture,
            elysia::core::Rect(cursor_x,render_y,glyph.render_width,glyph.render_height)
        );
        apply_opacity(command);
        out_commands.push_back(command);

        cursor_x += glyph.advance;
        if (index + 1 < glyphs.size())
            cursor_x += _digit_spacing;
    }
}

void UiNumber::set_value(double value)
{
    _value = value;
}

double UiNumber::value() const noexcept
{
    return _value;
}

void UiNumber::set_style(const UiNumberStyle& style) noexcept
{
    _style = style;
}

const UiNumberStyle& UiNumber::style() const noexcept
{
    return _style;
}

void UiNumber::set_text_color(elysia::core::Color color)
{
    _style.text = color;
}

elysia::core::Color UiNumber::text_color() const noexcept
{
    return _style.text;
}

void UiNumber::set_background_color(elysia::core::Color color)
{
    _style.background = color;
}

elysia::core::Color UiNumber::background_color() const noexcept
{
    return _style.background;
}

void UiNumber::set_draw_background(bool draw_background)
{
    _style.draw_background = draw_background;
}

bool UiNumber::draws_background() const noexcept
{
    return _style.draw_background;
}

void UiNumber::set_horizontal_align(TextHorizontalAlign align)
{
    _horizontal_align = align;
}

TextHorizontalAlign UiNumber::horizontal_align() const noexcept
{
    return _horizontal_align;
}

void UiNumber::set_vertical_align(TextVerticalAlign align)
{
    _vertical_align = align;
}

TextVerticalAlign UiNumber::vertical_align() const noexcept
{
    return _vertical_align;
}

void UiNumber::set_text_point_size(int point_size)
{
    _text_point_size = std::max(0,point_size);
}

int UiNumber::text_point_size() const noexcept
{
    return _text_point_size;
}

void UiNumber::set_padding(int padding)
{
    _padding = std::max(0,padding);
}

int UiNumber::padding() const noexcept
{
    return _padding;
}

void UiNumber::set_digit_spacing(float spacing)
{
    _digit_spacing = std::max(0.0f,spacing);
}

float UiNumber::digit_spacing() const noexcept
{
    return _digit_spacing;
}

void UiNumber::set_fixed_glyph_advance(float advance)
{
    _fixed_glyph_advance = std::max(0.0f,advance);
}

std::optional<float> UiNumber::fixed_glyph_advance() const noexcept
{
    return _fixed_glyph_advance;
}

void UiNumber::clear_fixed_glyph_advance()
{
    _fixed_glyph_advance.reset();
}

void UiNumber::set_target_height(float height)
{
    _target_height = std::max(0.0f,height);
}

std::optional<float> UiNumber::target_height() const noexcept
{
    return _target_height;
}

void UiNumber::clear_target_height()
{
    _target_height.reset();
}

void UiNumber::set_decimal_places(int decimal_places)
{
    _decimal_places = std::max(0,decimal_places);
}

int UiNumber::decimal_places() const noexcept
{
    return _decimal_places;
}

void UiNumber::set_trim_trailing_zeros(bool trim_trailing_zeros)
{
    _trim_trailing_zeros = trim_trailing_zeros;
}

bool UiNumber::trims_trailing_zeros() const noexcept
{
    return _trim_trailing_zeros;
}

void UiNumber::set_keep_decimal_point(bool keep_decimal_point)
{
    _keep_decimal_point = keep_decimal_point;
}

bool UiNumber::keeps_decimal_point() const noexcept
{
    return _keep_decimal_point;
}

void UiNumber::set_suffix(UiNumberSuffix suffix)
{
    _suffix = suffix;
}

UiNumberSuffix UiNumber::suffix() const noexcept
{
    return _suffix;
}

elysia::core::Rect UiNumber::content_rect() const noexcept
{
    const elysia::core::Rect& number_rect = screen_rect();
    const float width = std::max(0.0f,number_rect.width());
    const float height = std::max(0.0f,number_rect.height());
    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding,width * 0.5f);
    const float pad_y = std::min(padding,height * 0.5f);

    elysia::core::Rect content = number_rect;
    content.set_x(number_rect.x() + pad_x);
    content.set_y(number_rect.y() + pad_y);
    content.set_width(width - pad_x * 2.0f);
    content.set_height(height - pad_y * 2.0f);
    return content;
}

std::string UiNumber::formatted_text() const
{
    double display_value = _value;
    if (std::abs(display_value) < kNegativeZeroTolerance)
        display_value = 0.0;

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(_decimal_places) << display_value;
    std::string text = stream.str();

    if (_trim_trailing_zeros)
        text = trim_fractional_zeros(std::move(text),_keep_decimal_point);

    if (_suffix == UiNumberSuffix::Percent)
        text.push_back('%');

    return text;
}

std::string UiNumber::trim_fractional_zeros(std::string text,bool keep_decimal_point)
{
    if (text.find('.') == std::string::npos)
        return text;

    while (!text.empty() && text.back() == '0')
        text.pop_back();

    if (!text.empty() && text.back() == '.')
    {
        if (!keep_decimal_point)
            text.pop_back();
    }

    if (text == "-0" || text == "-0.")
        return keep_decimal_point && !text.empty() && text.back() == '.' ? "0." : "0";

    return text;
}

elysia::number::DigitAlignment UiNumber::digit_alignment() const noexcept
{
    switch (_horizontal_align)
    {
    case TextHorizontalAlign::Center:
        return elysia::number::DigitAlignment::Center;
    case TextHorizontalAlign::Right:
        return elysia::number::DigitAlignment::Right;
    case TextHorizontalAlign::Left:
    default:
        return elysia::number::DigitAlignment::Left;
    }
}
}
