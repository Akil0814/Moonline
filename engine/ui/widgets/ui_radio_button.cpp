#include "ui_radio_button.h"

#include "../../audio/audio_service.h"
#include "../style/ui_style_defaults.h"
#include "../style/ui_theme.h"

#include <utility>

namespace elysia::ui
{
UiRadioButton::UiRadioButton(const elysia::core::Rect& rect,int order) noexcept
    : UiControl(rect,order),_checkbox(rect,order)
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

    _checkbox.reset();
    _checkbox.set_use_theme(false);

    _on_selected = nullptr;
    _sounds.reset();
    _style_state.reset(UiStyleDefaults::radio_button());
    _text_key.clear();
    _label_placement = UiRadioButtonLabelPlacement::Right;
    _label_spacing = 8.0f;
    _text_placement = UiRadioButtonTextPlacement::NearIndicator;
    _text_color = style().text.enabled;
    _disabled_text_color = style().text.disabled;
    _text_point_size = _checkbox.text_point_size();
    _padding = _checkbox.padding();
    _label_padding = _checkbox.label_padding();
    _selected = false;
    _draw_background = false;
    _draw_border = false;
}

void UiRadioButton::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    _checkbox.set_enabled(enabled);
}

void UiRadioButton::set_focused(bool focused)
{
    UiControl::set_focused(focused);
    _checkbox.set_focused(focused);
}

bool UiRadioButton::on_ui_input_event(const UiInputEvent& event)
{
    sync_checkbox_state();

    const bool was_selected = _selected;
    const bool handled = _checkbox.on_ui_input_event(event);
    const bool checkbox_checked = _checkbox.is_checked();

    if (!was_selected && checkbox_checked)
        (void)set_selected_internal(true,true);
    else if (was_selected && !checkbox_checked)
        _checkbox.set_checked(true);

    sync_checkbox_state();
    return handled || (was_selected && !checkbox_checked);
}

void UiRadioButton::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    sync_checkbox_state();
    _checkbox.submit_ui_render_commands(out_commands);
}

void UiRadioButton::set_radio_button_config(const UiRadioButtonConfig& config)
{
    apply_radio_button_config(config);
}

void UiRadioButton::set_selected(bool selected) noexcept
{
    (void)set_selected_internal(selected,false);
    sync_checkbox_state();
}

bool UiRadioButton::is_selected() const noexcept
{
    return _selected;
}

void UiRadioButton::select()
{
    (void)set_selected_internal(true,true);
    sync_checkbox_state();
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

void UiRadioButton::set_style(const UiRadioButtonStyle& style) noexcept
{
    _style_state.set_style_override(style);
    _text_color = this->style().text.enabled;
    _disabled_text_color = this->style().text.disabled;
}

const UiRadioButtonStyle& UiRadioButton::style() const noexcept
{
    return _style_state.effective_style();
}

bool UiRadioButton::has_style_override() const noexcept
{
    return _style_state.has_style_override();
}

void UiRadioButton::clear_style_override() noexcept
{
    _style_state.clear_style_override();
    _text_color = style().text.enabled;
    _disabled_text_color = style().text.disabled;
}

void UiRadioButton::apply_radio_button_config(const UiRadioButtonConfig& config)
{
    _sounds = config.sounds;
    if (config.style)
        set_style(*config.style);

    _text_key = config.text_key;
    _label_placement = config.label_placement;
    _label_spacing = config.label_spacing;
    _text_placement = config.text_placement;
    if (config.text_color)
        _text_color = *config.text_color;
    if (config.disabled_text_color)
        _disabled_text_color = *config.disabled_text_color;
    if (config.text_point_size)
        _text_point_size = *config.text_point_size;
    if (config.padding)
        _padding = *config.padding;
    if (config.label_padding)
        _label_padding = *config.label_padding;
    _draw_background = config.draw_background;
    _draw_border = config.draw_border;

    notify_layout_parent_of_intrinsic_layout_invalidation();
}

void UiRadioButton::sync_checkbox_state() const
{
    UiLabeledCheckboxConfig config{};
    config.text_key = _text_key;
    config.label_placement = to_checkbox_label_placement(_label_placement);
    config.label_spacing = _label_spacing;
    config.text_placement = to_checkbox_text_placement(_text_placement);
    config.text_colors = UiEnabledDisabledColors{ _text_color,_disabled_text_color };
    config.draw_background = _draw_background;
    config.draw_border = _draw_border;

    UiCheckboxConfig checkbox_config{};
    checkbox_config.style = checkbox_style();
    if (_sounds)
        checkbox_config.sounds = checkbox_sounds();
    config.checkbox = checkbox_config;

    _checkbox.set_screen_rect(screen_rect());
    _checkbox.set_visible(is_visible());
    _checkbox.set_active(is_active());
    _checkbox.set_enabled(is_enabled());
    _checkbox.set_focused(is_focused());
    _checkbox.set_opacity(opacity());
    _checkbox.set_labeled_checkbox_config(config);
    _checkbox.set_text_point_size(_text_point_size);
    _checkbox.set_padding(_padding);
    _checkbox.set_label_padding(_label_padding);
    _checkbox.set_checked(_selected);
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

void UiRadioButton::play_sound_if_set(const std::string& sound_key) const
{
    if (sound_key.empty())
        return;
    elysia::audio::AudioService::instance()->play_sound(sound_key);
}

UiCheckboxStyle UiRadioButton::checkbox_style() const noexcept
{
    UiCheckboxStyle style{};
    style.chrome = this->style().chrome;
    style.mark = this->style().mark;
    style.mark_style = UiCheckboxMarkStyle::RadioDot;
    return style;
}

UiCheckboxSounds UiRadioButton::checkbox_sounds() const noexcept
{
    UiCheckboxSounds sounds{};
    if (_sounds)
    {
        sounds.focus = _sounds->focus;
        sounds.press = _sounds->press;
    }
    return sounds;
}

UiLabeledCheckboxLabelPlacement UiRadioButton::to_checkbox_label_placement(UiRadioButtonLabelPlacement placement) noexcept
{
    return placement == UiRadioButtonLabelPlacement::Left
        ? UiLabeledCheckboxLabelPlacement::Left
        : UiLabeledCheckboxLabelPlacement::Right;
}

UiLabeledCheckboxTextPlacement UiRadioButton::to_checkbox_text_placement(UiRadioButtonTextPlacement placement) noexcept
{
    return placement == UiRadioButtonTextPlacement::FarEdge
        ? UiLabeledCheckboxTextPlacement::FarEdge
        : UiLabeledCheckboxTextPlacement::NearBox;
}

void UiRadioButton::apply_theme(const UiTheme& theme)
{
    // The outer radio button owns theme participation and translates it into local text colors.
    // Its internal checkbox remains an implementation detail rather than a separately registered
    // themed element.
    _style_state.set_theme_style(theme.radio_button_style);
    _text_color = style().text.enabled;
    _disabled_text_color = style().text.disabled;
}
}
