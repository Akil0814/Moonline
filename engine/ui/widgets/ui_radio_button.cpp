#include "ui_radio_button.h"

#include "../../audio/audio_service.h"
#include "../../core/render/render_command.h"
#include "../style/ui_style_defaults.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace elysia::ui
{
namespace
{
[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}

void submit_fill_disc(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    const elysia::core::Rect& bounds,
    elysia::core::Color color
)
{
    if (bounds.is_empty())
        return;

    const float diameter = std::min(bounds.width(),bounds.height());
    if (diameter <= 0.0f)
        return;

    const float radius = diameter * 0.5f;
    const float center_x = bounds.center().x;
    const float center_y = bounds.center().y;
    const int slice_count = std::max(1,static_cast<int>(std::round(diameter)));
    const float slice_height = diameter / static_cast<float>(slice_count);

    for (int index = 0; index < slice_count; ++index)
    {
        const float y0 = center_y - radius + slice_height * static_cast<float>(index);
        const float y1 = std::min(center_y + radius,y0 + slice_height);
        const float sample_y = std::min(center_y + radius,std::max(center_y - radius,y0 + (y1 - y0) * 0.5f));
        const float distance_y = sample_y - center_y;
        const float distance_x = std::sqrt(std::max(0.0f,radius * radius - distance_y * distance_y));
        const elysia::core::Rect slice(
            center_x - distance_x,
            y0,
            distance_x * 2.0f,
            std::max(1.0f,y1 - y0)
        );
        if (!slice.is_empty())
            out_commands.push_back(elysia::core::make_ui_fill_rect_command(slice,color));
    }
}
}

UiRadioButton::UiRadioButton(const elysia::core::Rect& rect,int order) noexcept
    : UiControl(rect,order)
{
    reset();
}

UiRadioButton::UiRadioButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiRadioButton(elysia::core::Rect(position.x,position.y,size.x,size.y),order) {}

UiRadioButton::UiRadioButton(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiRadioButton(elysia::core::Rect::from_center(center,size),order) {}

UiRadioButton::UiRadioButton(const elysia::core::Rect& rect,const UiRadioButtonConfig& config,int order) noexcept
    : UiRadioButton(rect,order)
{
    set_radio_button_config(config);
}

UiRadioButton::UiRadioButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiRadioButtonConfig& config,int order) noexcept
    : UiRadioButton(elysia::core::Rect(position.x,position.y,size.x,size.y),config,order) {}

UiRadioButton::UiRadioButton(
    const elysia::core::Vector2& center,const elysia::core::Vector2& size,
    UiFromCenterTag,const UiRadioButtonConfig& config,int order
) noexcept : UiRadioButton(elysia::core::Rect::from_center(center,size),config,order) {}

void UiRadioButton::reset() noexcept
{
    UiControl::reset();
    set_use_theme(false);

    _label.reset();
    _label.set_use_theme(false);
    _label.set_draw_background(false);
    _label.set_horizontal_align(TextHorizontalAlign::Left);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_padding(0);

    _on_selected = nullptr;
    _sounds.reset();
    _style = UiStyleDefaults::radio_button();
    _text_key.clear();
    _label_placement = UiRadioButtonLabelPlacement::Right;
    _label_spacing = 8.0f;
    _text_placement = UiRadioButtonTextPlacement::NearIndicator;
    _padding = 4;
    _selected = false;
    _draw_background = false;
    _draw_border = false;
    _is_pushed = false;
}

void UiRadioButton::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    if (!enabled)
        clear_pushed_state();
}

void UiRadioButton::set_focused(bool focused)
{
    const bool was_focused = is_focused();
    UiControl::set_focused(focused);
    if (!is_focused())
        clear_pushed_state();
    if (!was_focused && is_focused() && _sounds)
        play_sound_if_set(_sounds->focus);
}

bool UiRadioButton::on_ui_input_event(const UiInputEvent& event)
{
    if (event.type == UiInputEventType::MouseMoved)
    {
        if (!can_receive_pointer())
        {
            set_focused(false);
            clear_pushed_state();
            return false;
        }

        set_focused(contains_pointer(event.mouse_x,event.mouse_y));
        return false;
    }

    if (event.type == UiInputEventType::PointerPressed)
    {
        if (!is_primary_pointer_event(event) || !can_receive_pointer())
            return false;
        if (!contains_pointer(event.mouse_x,event.mouse_y))
            return false;

        set_focused(true);
        _is_pushed = true;
        if (_sounds)
            play_sound_if_set(_sounds->press);
        return true;
    }

    if (event.type == UiInputEventType::PointerReleased)
    {
        if (!is_primary_pointer_event(event))
            return false;

        const bool was_pushed = _is_pushed;
        const bool is_inside = can_receive_pointer() && contains_pointer(event.mouse_x,event.mouse_y);
        set_focused(is_inside);
        clear_pushed_state();

        if (was_pushed && is_inside)
        {
            if (_selected)
                return true;
            return set_selected_internal(true,true);
        }

        return was_pushed;
    }

    if (event.action != UiAction::Confirm)
        return false;

    if (!can_interact())
    {
        clear_pushed_state();
        return false;
    }

    if (event.type == UiInputEventType::ActionPressed)
    {
        _is_pushed = true;
        if (_sounds)
            play_sound_if_set(_sounds->press);
        return true;
    }

    if (event.type == UiInputEventType::ActionReleased)
    {
        const bool should_select = _is_pushed;
        clear_pushed_state();
        if (!should_select)
            return false;
        if (_selected)
            return true;
        return set_selected_internal(true,true);
    }

    return false;
}

void UiRadioButton::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& rect = screen_rect();
    if (!rect.is_empty())
    {
        if (_draw_background)
            out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(current_background_color())));
        if (_draw_border)
            out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(current_border_color())));
    }

    const elysia::core::Rect indicator = indicator_rect();
    if (!indicator.is_empty())
    {
        if (_style.chrome.draw_background)
            submit_fill_disc(out_commands,indicator,apply_opacity(current_background_color()));
        if (_style.chrome.draw_border)
            submit_fill_disc(out_commands,indicator,apply_opacity(current_border_color()));

        const float inset = std::max(2.0f,std::min(indicator.width(),indicator.height()) * 0.14f);
        const elysia::core::Rect inner_indicator(
            indicator.x() + inset,
            indicator.y() + inset,
            std::max(0.0f,indicator.width() - inset * 2.0f),
            std::max(0.0f,indicator.height() - inset * 2.0f)
        );
        if (_style.chrome.draw_background && !inner_indicator.is_empty())
            submit_fill_disc(out_commands,inner_indicator,apply_opacity(current_background_color()));

        if (_selected)
        {
            const float dot_inset = std::max(4.0f,std::min(indicator.width(),indicator.height()) * 0.32f);
            const elysia::core::Rect dot_rect(
                indicator.x() + dot_inset,
                indicator.y() + dot_inset,
                std::max(0.0f,indicator.width() - dot_inset * 2.0f),
                std::max(0.0f,indicator.height() - dot_inset * 2.0f)
            );
            if (!dot_rect.is_empty())
                submit_fill_disc(out_commands,dot_rect,apply_opacity(current_mark_color()));
        }
    }

    sync_label_visuals();
    _label.submit_ui_render_commands(out_commands);
}

void UiRadioButton::set_radio_button_config(const UiRadioButtonConfig& config)
{
    apply_radio_button_config(config);
}

void UiRadioButton::set_selected(bool selected) noexcept
{
    (void)set_selected_internal(selected,false);
}

bool UiRadioButton::is_selected() const noexcept
{
    return _selected;
}

void UiRadioButton::select()
{
    (void)set_selected_internal(true,true);
}

void UiRadioButton::set_on_selected(UiRadioButtonSelectedCallback on_selected)
{
    _on_selected = std::move(on_selected);
}

void UiRadioButton::set_text_key(std::string text_key)
{
    _text_key = std::move(text_key);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const std::string& UiRadioButton::text_key() const noexcept
{
    return _text_key;
}

void UiRadioButton::set_label_placement(UiRadioButtonLabelPlacement placement) noexcept
{
    _label_placement = placement;
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiRadioButtonLabelPlacement UiRadioButton::label_placement() const noexcept
{
    return _label_placement;
}

void UiRadioButton::set_label_spacing(float spacing) noexcept
{
    _label_spacing = clamp_non_negative(spacing);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

float UiRadioButton::label_spacing() const noexcept
{
    return _label_spacing;
}

void UiRadioButton::set_text_placement(UiRadioButtonTextPlacement placement) noexcept
{
    _text_placement = placement;
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiRadioButtonTextPlacement UiRadioButton::text_placement() const noexcept
{
    return _text_placement;
}

void UiRadioButton::set_style(const UiRadioButtonStyle& style) noexcept
{
    _style = style;
}

const UiRadioButtonStyle& UiRadioButton::style() const noexcept
{
    return _style;
}

void UiRadioButton::set_text_color(elysia::core::Color color) noexcept
{
    _style.text.enabled = color;
}

elysia::core::Color UiRadioButton::text_color() const noexcept
{
    return _style.text.enabled;
}

void UiRadioButton::set_disabled_text_color(elysia::core::Color color) noexcept
{
    _style.text.disabled = color;
}

elysia::core::Color UiRadioButton::disabled_text_color() const noexcept
{
    return _style.text.disabled;
}

void UiRadioButton::set_sounds(const UiRadioButtonSounds& sounds)
{
    _sounds = sounds;
}

void UiRadioButton::clear_sounds() noexcept
{
    _sounds.reset();
}

const std::optional<UiRadioButtonSounds>& UiRadioButton::sounds() const noexcept
{
    return _sounds;
}

void UiRadioButton::set_text_point_size(int point_size) noexcept
{
    _label.set_text_point_size(point_size);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

int UiRadioButton::text_point_size() const noexcept
{
    return _label.text_point_size();
}

void UiRadioButton::set_padding(int padding) noexcept
{
    _padding = std::max(0,padding);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

int UiRadioButton::padding() const noexcept
{
    return _padding;
}

void UiRadioButton::set_label_padding(int padding) noexcept
{
    _label.set_padding(padding);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

int UiRadioButton::label_padding() const noexcept
{
    return _label.padding();
}

void UiRadioButton::set_draw_background(bool draw_background) noexcept
{
    _draw_background = draw_background;
}

bool UiRadioButton::draws_background() const noexcept
{
    return _draw_background;
}

void UiRadioButton::set_draw_border(bool draw_border) noexcept
{
    _draw_border = draw_border;
}

bool UiRadioButton::draws_border() const noexcept
{
    return _draw_border;
}

void UiRadioButton::apply_radio_button_config(const UiRadioButtonConfig& config)
{
    if (config.sounds)
        set_sounds(*config.sounds);
    else
        clear_sounds();

    if (config.style)
        set_style(*config.style);

    set_text_key(config.text_key);
    set_label_placement(config.label_placement);
    set_label_spacing(config.label_spacing);
    set_text_placement(config.text_placement);
    if (config.text_color)
        set_text_color(*config.text_color);
    if (config.disabled_text_color)
        set_disabled_text_color(*config.disabled_text_color);
    if (config.text_point_size)
        set_text_point_size(*config.text_point_size);
    if (config.padding)
        set_padding(*config.padding);
    if (config.label_padding)
        set_label_padding(*config.label_padding);
    set_draw_background(config.draw_background);
    set_draw_border(config.draw_border);
}

bool UiRadioButton::set_selected_internal(bool selected,bool notify) noexcept
{
    if (_selected == selected)
        return false;

    _selected = selected;
    if (_selected && notify && _on_selected)
        _on_selected();
    if (_selected && notify && _sounds)
        play_sound_if_set(_sounds->select);
    return true;
}

bool UiRadioButton::can_interact() const noexcept
{
    return is_enabled() && is_focused() && is_active() && is_visible();
}

bool UiRadioButton::can_receive_pointer() const noexcept
{
    return is_enabled() && is_active() && is_visible();
}

bool UiRadioButton::contains_pointer(int mouse_x,int mouse_y) const noexcept
{
    return screen_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

bool UiRadioButton::is_primary_pointer_event(const UiInputEvent& event) const noexcept
{
    return event.device == elysia::input::InputDevice::Mouse
        && event.control == elysia::input::RawInputControl::MouseLeft;
}

void UiRadioButton::clear_pushed_state() noexcept
{
    _is_pushed = false;
}

void UiRadioButton::play_sound_if_set(const std::string& sound_key) const
{
    if (sound_key.empty())
        return;
    elysia::audio::AudioService::instance()->play_sound(sound_key);
}

void UiRadioButton::sync_label_visuals() const
{
    UiLabelStyle label_style = UiStyleDefaults::label();
    label_style.text = current_text_color();
    label_style.draw_background = false;

    _label.set_screen_rect(label_rect());
    _label.set_visible(!_text_key.empty() && !_label.screen_rect().is_empty() && is_visible());
    _label.set_opacity(opacity());
    _label.set_text_key(_text_key);
    _label.set_style(label_style);
    _label.set_vertical_align(TextVerticalAlign::Center);
    if (_label_placement == UiRadioButtonLabelPlacement::Left)
    {
        _label.set_horizontal_align(
            _text_placement == UiRadioButtonTextPlacement::NearIndicator
                ? TextHorizontalAlign::Right
                : TextHorizontalAlign::Left);
    }
    else
    {
        _label.set_horizontal_align(
            _text_placement == UiRadioButtonTextPlacement::NearIndicator
                ? TextHorizontalAlign::Left
                : TextHorizontalAlign::Right);
    }
}

elysia::core::Rect UiRadioButton::content_rect() const noexcept
{
    const elysia::core::Rect& control_rect = screen_rect();
    const float width = std::max(0.0f,control_rect.width());
    const float height = std::max(0.0f,control_rect.height());
    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding,width * 0.5f);
    const float pad_y = std::min(padding,height * 0.5f);

    elysia::core::Rect content = control_rect;
    content.set_x(control_rect.x() + pad_x);
    content.set_y(control_rect.y() + pad_y);
    content.set_width(width - pad_x * 2.0f);
    content.set_height(height - pad_y * 2.0f);
    return content;
}

elysia::core::Rect UiRadioButton::indicator_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    if (content.is_empty())
        return elysia::core::Rect::zero();

    const float side = std::min(content.width(),content.height());
    if (side <= 0.0f)
        return elysia::core::Rect::zero();

    const float y = content.center().y - side * 0.5f;
    if (_label_placement == UiRadioButtonLabelPlacement::Left)
        return elysia::core::Rect(content.right() - side,y,side,side);
    return elysia::core::Rect(content.x(),y,side,side);
}

elysia::core::Rect UiRadioButton::label_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    const elysia::core::Rect indicator = indicator_rect();
    if (content.is_empty() || indicator.is_empty())
        return elysia::core::Rect::zero();

    const float spacing = _text_key.empty() ? 0.0f : _label_spacing;
    if (_label_placement == UiRadioButtonLabelPlacement::Left)
    {
        const float width = std::max(0.0f,indicator.x() - content.x() - spacing);
        return elysia::core::Rect(content.x(),content.y(),width,content.height());
    }

    const float x = indicator.right() + spacing;
    const float width = std::max(0.0f,content.right() - x);
    return elysia::core::Rect(x,content.y(),width,content.height());
}

elysia::core::Color UiRadioButton::current_background_color() const noexcept
{
    return resolve_interactive_color(_style.chrome.background,is_enabled(),is_focused(),_is_pushed);
}

elysia::core::Color UiRadioButton::current_border_color() const noexcept
{
    return resolve_enabled_disabled_color(_style.chrome.border,is_enabled());
}

elysia::core::Color UiRadioButton::current_mark_color() const noexcept
{
    return resolve_enabled_disabled_color(_style.mark,is_enabled());
}

elysia::core::Color UiRadioButton::current_text_color() const noexcept
{
    return resolve_enabled_disabled_color(_style.text,is_enabled());
}
}
