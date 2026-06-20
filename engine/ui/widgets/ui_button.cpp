#include "ui_button.h"

#include "../../core/render/colors.h"
#include "../../core/render/render_command.h"
#include "../../localization/localization_manager.h"
#include "../../localization/localized_text_style.h"

#include <algorithm>
#include <utility>

UiButton::UiButton(const Rect& rect, int order, std::string text_key) noexcept
    : UiControl(rect.position(), rect.size(), order),
      _text_key(std::move(text_key))
{
    set_use_theme(false);
}

UiButton::UiButton(
    const Vector2& position,
    const Vector2& size,
    int order,
    std::string text_key
) noexcept
    : UiControl(position, size, order),
      _text_key(std::move(text_key))
{
    set_use_theme(false);
}

void UiButton::reset() noexcept
{
    UiControl::reset();
    set_use_theme(false);

    _on_click = nullptr;
    _idle_color = colors::loading_blue_button_idle;
    _focused_color = colors::loading_blue_button_hovered;
    _pushed_color = colors::loading_blue_button_pushed;
    _border_color = colors::loading_blue_button_frame;
    _text_color = colors::white;
    _text_point_size = 24;
    _padding = 10;
    _draw_border = true;
    _is_pushed = false;
}

void UiButton::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    if (!enabled)
    {
        clear_pushed_state();
    }
}

void UiButton::set_focused(bool focused)
{
    UiControl::set_focused(focused);
    if (!focused)
    {
        clear_pushed_state();
    }
}

bool UiButton::on_ui_input_event(const UiInputEvent& event)
{
    if (event.action != UiAction::Confirm)
    {
        return false;
    }

    if (!can_interact())
    {
        clear_pushed_state();
        return false;
    }

    if (event.type == UiInputEventType::ActionPressed)
    {
        _is_pushed = true;
        return true;
    }

    if (event.type == UiInputEventType::ActionReleased)
    {
        const bool should_click = _is_pushed;
        clear_pushed_state();

        if (should_click && _on_click)
        {
            _on_click();
        }

        return should_click;
    }

    return false;
}

void UiButton::submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const
{
    if (!is_visible())
    {
        return;
    }

    const Rect& button_rect = screen_rect();
    if (button_rect.is_empty())
    {
        return;
    }

    out_commands.push_back(make_ui_fill_rect_command(button_rect, current_background_color()));

    if (_draw_border)
    {
        out_commands.push_back(make_ui_draw_rect_command(button_rect, _border_color));
    }

    if (_text_key.empty())
    {
        return;
    }

    LocalizationManager* localization_manager = LocalizationManager::instance();
    if (!localization_manager)
    {
        return;
    }

    LocalizedTextStyle text_style;
    text_style.point_size = _text_point_size;
    text_style.color = _text_color;
    text_style.wrap_width = std::max(0, static_cast<int>(content_rect().width()));

    SDL_Texture* text_texture = localization_manager->get_text_texture(_text_key, text_style);
    if (!text_texture)
    {
        return;
    }

    const Rect text_rect = text_render_rect(text_texture);
    if (text_rect.is_empty())
    {
        return;
    }

    out_commands.push_back(make_ui_texture_command(text_texture, text_rect));
}

void UiButton::set_text_key(std::string text_key)
{
    _text_key = std::move(text_key);
}

const std::string& UiButton::text_key() const noexcept
{
    return _text_key;
}

void UiButton::set_on_click(ClickCallback on_click)
{
    _on_click = std::move(on_click);
}

void UiButton::set_idle_color(Color color)
{
    _idle_color = color;
}

Color UiButton::idle_color() const noexcept
{
    return _idle_color;
}

void UiButton::set_focused_color(Color color)
{
    _focused_color = color;
}

Color UiButton::focused_color() const noexcept
{
    return _focused_color;
}

void UiButton::set_pushed_color(Color color)
{
    _pushed_color = color;
}

Color UiButton::pushed_color() const noexcept
{
    return _pushed_color;
}

void UiButton::set_border_color(Color color)
{
    _border_color = color;
}

Color UiButton::border_color() const noexcept
{
    return _border_color;
}

void UiButton::set_text_color(Color color)
{
    _text_color = color;
}

Color UiButton::text_color() const noexcept
{
    return _text_color;
}

void UiButton::set_text_point_size(int point_size)
{
    _text_point_size = std::max(0, point_size);
}

int UiButton::text_point_size() const noexcept
{
    return _text_point_size;
}

void UiButton::set_padding(int padding)
{
    _padding = std::max(0, padding);
}

int UiButton::padding() const noexcept
{
    return _padding;
}

void UiButton::set_draw_border(bool draw_border)
{
    _draw_border = draw_border;
}

bool UiButton::draws_border() const noexcept
{
    return _draw_border;
}

bool UiButton::can_interact() const noexcept
{
    return is_enabled() && is_focused() && is_active() && is_visible();
}

Color UiButton::current_background_color() const noexcept
{
    if (_is_pushed)
    {
        return _pushed_color;
    }

    if (is_focused())
    {
        return _focused_color;
    }

    return _idle_color;
}

Rect UiButton::content_rect() const noexcept
{
    const Rect& button_rect = screen_rect();
    const float width = std::max(0.0f, button_rect.width());
    const float height = std::max(0.0f, button_rect.height());

    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding, width * 0.5f);
    const float pad_y = std::min(padding, height * 0.5f);

    Rect content = button_rect;
    content.set_x(button_rect.x() + pad_x);
    content.set_y(button_rect.y() + pad_y);
    content.set_width(width - pad_x * 2.0f);
    content.set_height(height - pad_y * 2.0f);
    return content;
}

Rect UiButton::text_render_rect(SDL_Texture* text_texture) const noexcept
{
    if (!text_texture)
    {
        return Rect::zero();
    }

    int texture_width = 0;
    int texture_height = 0;
    if (SDL_QueryTexture(text_texture, nullptr, nullptr, &texture_width, &texture_height) != 0)
    {
        return Rect::zero();
    }

    if (texture_width <= 0 || texture_height <= 0)
    {
        return Rect::zero();
    }

    const Rect available_rect = content_rect();
    if (available_rect.is_empty())
    {
        return Rect::zero();
    }

    const float available_width = available_rect.width();
    const float available_height = available_rect.height();
    const float width_scale = available_width / static_cast<float>(texture_width);
    const float height_scale = available_height / static_cast<float>(texture_height);
    const float scale = std::min(1.0f, std::min(width_scale, height_scale));

    const Vector2 render_size(
        static_cast<float>(texture_width) * scale,
        static_cast<float>(texture_height) * scale
    );

    return Rect::from_center(available_rect.center(), render_size);
}

void UiButton::clear_pushed_state() noexcept
{
    _is_pushed = false;
}
