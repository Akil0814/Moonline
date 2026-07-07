#include "ui_slider.h"

#include "../../audio/audio_service.h"
#include "../../core/render/render_command.h"
#include "../../localization/localization_manager.h"
#include "../../localization/localized_text_style.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

namespace elysia::ui
{
namespace
{
constexpr float kValueChangeEpsilon = 0.0001f;

[[nodiscard]] bool nearly_equal(float a,float b) noexcept
{
    return std::fabs(a - b) <= kValueChangeEpsilon;
}

[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}

[[nodiscard]] UiDragHandleConfig make_drag_handle_config(const UiSliderHandleStyle& style) noexcept
{
    UiDragHandleConfig config{};
    config.size = elysia::core::Vector2(clamp_non_negative(style.size.x),clamp_non_negative(style.size.y));
    config.textures = style.textures;
    config.draw_background = style.draw_background;
    config.draw_border = style.draw_border;
    config.idle_color = style.idle_color;
    config.focused_color = style.focused_color;
    config.dragging_color = style.dragging_color;
    config.disabled_background_color = style.disabled_background_color;
    config.border_color = style.border_color;
    config.disabled_border_color = style.disabled_border_color;
    return config;
}

[[nodiscard]] elysia::core::Rect take_left(elysia::core::Rect& rect,float width) noexcept
{
    width = std::clamp(width,0.0f,std::max(0.0f,rect.width()));
    const elysia::core::Rect slot(rect.x(),rect.y(),width,rect.height());
    rect.set_x(rect.x() + width);
    rect.set_width(std::max(0.0f,rect.width() - width));
    return slot;
}

[[nodiscard]] elysia::core::Rect take_right(elysia::core::Rect& rect,float width) noexcept
{
    width = std::clamp(width,0.0f,std::max(0.0f,rect.width()));
    const elysia::core::Rect slot(rect.right() - width,rect.y(),width,rect.height());
    rect.set_width(std::max(0.0f,rect.width() - width));
    return slot;
}

[[nodiscard]] elysia::core::Rect take_top(elysia::core::Rect& rect,float height) noexcept
{
    height = std::clamp(height,0.0f,std::max(0.0f,rect.height()));
    const elysia::core::Rect slot(rect.x(),rect.y(),rect.width(),height);
    rect.set_y(rect.y() + height);
    rect.set_height(std::max(0.0f,rect.height() - height));
    return slot;
}

[[nodiscard]] elysia::core::Rect take_bottom(elysia::core::Rect& rect,float height) noexcept
{
    height = std::clamp(height,0.0f,std::max(0.0f,rect.height()));
    const elysia::core::Rect slot(rect.x(),rect.bottom() - height,rect.width(),height);
    rect.set_height(std::max(0.0f,rect.height() - height));
    return slot;
}

[[nodiscard]] float preferred_side_slot_extent(const elysia::core::Rect& rect,const elysia::core::Vector2& handle_size) noexcept
{
    return std::min(std::max(56.0f,handle_size.x * 2.0f),std::max(0.0f,rect.width() * 0.35f));
}

[[nodiscard]] float preferred_vertical_slot_extent(
    const elysia::core::Rect& rect,
    const elysia::core::Vector2& handle_size,
    float target_height
) noexcept
{
    return std::min(std::max({ 24.0f,handle_size.y,target_height }) + 8.0f,std::max(0.0f,rect.height() * 0.35f));
}
}

struct UiSlider::SliderLayout
{
    elysia::core::Rect track_area = elysia::core::Rect::zero();
    elysia::core::Rect bar_rect = elysia::core::Rect::zero();
    elysia::core::Rect handle_rect = elysia::core::Rect::zero();
    elysia::core::Rect label_rect = elysia::core::Rect::zero();
    elysia::core::Rect value_rect = elysia::core::Rect::zero();
};

UiSlider::UiSlider(const elysia::core::Rect& rect,int order) noexcept
    : UiControl(rect,order)
{
    reset();
}

UiSlider::UiSlider(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiControl(position,size,order)
{
    reset();
}

UiSlider::UiSlider(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiControl(center,size,from_center,order)
{
    reset();
}

UiSlider::UiSlider(const elysia::core::Rect& rect,const UiSliderConfig& config,int order) noexcept
    : UiSlider(rect,order)
{
    set_slider_config(config);
}

UiSlider::UiSlider(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiSliderConfig& config,int order) noexcept
    : UiSlider(elysia::core::Rect(position.x,position.y,size.x,size.y),config,order) {}

UiSlider::UiSlider(
    const elysia::core::Vector2& center,const elysia::core::Vector2& size,
    UiFromCenterTag,const UiSliderConfig& config,int order
) noexcept : UiSlider(elysia::core::Rect::from_center(center,size),config,order) {}

void UiSlider::reset() noexcept
{
    UiControl::reset();
    _bar.reset();
    _handle.reset();
    _label.reset();
    _value_number.reset();
    set_use_theme(false);

    _text_key.clear();
    _icon = nullptr;
    _sounds.reset();
    _handle_style = UiSliderHandleStyle{};
    _on_value_changed = nullptr;
    _label_placement = UiSliderLabelPlacement::None;
    _orientation = UiSliderOrientation::Horizontal;
    _value_label_mode = UiSliderValueLabelMode::None;
    _draw_background = true;
    _draw_border = true;
    _drag_value_changed = false;
    _bar_thickness = 6.0f;
    _min_value = 0.0f;
    _max_value = 1.0f;
    _value = 0.0f;
    _step = std::nullopt;
    _last_slide_sound_ticks = 0;
    _has_last_slide_sound_tick = false;
    _background_color = elysia::core::colors::cobalt_blue;
    _disabled_background_color = elysia::core::colors::gray_700;
    _border_color = elysia::core::colors::sky_blue;
    _disabled_border_color = elysia::core::colors::gray_500;
    _fill_color = elysia::core::colors::glacial_white;
    _disabled_fill_color = elysia::core::colors::gray_500;
    _text_color = elysia::core::colors::white;
    _disabled_text_color = elysia::core::colors::gray_300;

    initialize_child_widgets();
}

void UiSlider::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    _handle.set_enabled(enabled);
    if (!enabled)
        clear_drag_state();
}

void UiSlider::set_focused(bool focused)
{
    const bool was_focused = is_focused();
    UiControl::set_focused(focused);
    _handle.set_focused(is_focused());
    if (!is_focused())
        clear_drag_state();
    if (!was_focused && is_focused() && _sounds)
        play_sound_if_set(_sounds->on_focus);
}
bool UiSlider::on_ui_input_event(const UiInputEvent& event)
{
    if (event.type == UiInputEventType::MouseMoved)
    {
        if (!can_receive_pointer())
        {
            set_focused(false);
            _handle.set_focused(false);
            clear_drag_state();
            return false;
        }

        if (!_handle.is_dragging())
            sync_child_rects(compute_layout());
        sync_child_visuals();
        const bool handle_handled = _handle.on_ui_input_event(event);
        set_focused(_handle.is_dragging() || contains_pointer(event.mouse_x,event.mouse_y));
        _handle.set_focused(is_focused());
        if (!_handle.is_dragging())
            sync_child_rects(compute_layout());
        return handle_handled;
    }

    if (event.type == UiInputEventType::PointerPressed)
    {
        if (!is_primary_pointer_event(event) || !can_receive_pointer())
            return false;

        const SliderLayout layout = compute_layout();
        if (!contains_track_or_handle(layout,event.mouse_x,event.mouse_y))
            return false;

        const elysia::core::Vector2 pointer(static_cast<float>(event.mouse_x),static_cast<float>(event.mouse_y));
        sync_child_rects(layout);
        sync_child_visuals();
        set_focused(true);
        _handle.set_focused(true);
        _drag_value_changed = false;
        _has_last_slide_sound_tick = false;
        if (!layout.handle_rect.contains(pointer))
        {
            _drag_value_changed = update_value_from_point(layout,pointer,true);
            sync_child_rects(compute_layout());
        }
        _handle.begin_drag_from_pointer(pointer);
        return true;
    }

    if (event.type == UiInputEventType::PointerReleased)
    {
        if (!is_primary_pointer_event(event))
            return false;

        const bool handle_handled = _handle.on_ui_input_event(event);
        sync_child_rects(compute_layout());
        sync_child_visuals();
        const bool is_inside = can_receive_pointer() && contains_pointer(event.mouse_x,event.mouse_y);
        set_focused(is_inside);
        _handle.set_focused(is_focused());
        return handle_handled;
    }

    if (event.type != UiInputEventType::ActionPressed)
        return false;
    if (!can_interact())
    {
        clear_drag_state();
        return false;
    }

    const float delta = action_step();
    float target_value = _value;
    bool handled = false;
    switch (event.action)
    {
    case UiAction::Home:
        target_value = _min_value;
        handled = true;
        break;
    case UiAction::End:
        target_value = _max_value;
        handled = true;
        break;
    case UiAction::NavigateLeft:
        if (_orientation == UiSliderOrientation::Horizontal)
        {
            target_value = _value - delta;
            handled = true;
        }
        break;
    case UiAction::NavigateRight:
        if (_orientation == UiSliderOrientation::Horizontal)
        {
            target_value = _value + delta;
            handled = true;
        }
        break;
    case UiAction::NavigateUp:
        if (_orientation == UiSliderOrientation::Vertical)
        {
            target_value = _value + delta;
            handled = true;
        }
        break;
    case UiAction::NavigateDown:
        if (_orientation == UiSliderOrientation::Vertical)
        {
            target_value = _value - delta;
            handled = true;
        }
        break;
    default:
        break;
    }

    if (!handled)
        return false;
    if (set_value_internal(target_value,true))
        play_slide_sound_if_allowed();
    return true;
}

void UiSlider::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& slider_rect = screen_rect();
    if (slider_rect.is_empty())
        return;

    const SliderLayout layout = compute_layout();
    if (_draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(slider_rect,apply_opacity(current_background_color())));

    sync_child_rects(layout);
    sync_child_visuals();
    sync_value_number_content();
    _bar.submit_ui_render_commands(out_commands);
    _handle.submit_ui_render_commands(out_commands);

    if (_icon && !layout.label_rect.is_empty())
    {
        const elysia::core::Rect icon_rect = fitted_texture_rect(layout.label_rect,_icon);
        if (!icon_rect.is_empty())
        {
            elysia::core::UiRenderCommand command = elysia::core::make_ui_texture_command(_icon,icon_rect);
            apply_opacity(command);
            out_commands.push_back(command);
        }
    }
    else
    {
        _label.submit_ui_render_commands(out_commands);
    }

    if (_value_label_mode != UiSliderValueLabelMode::None)
        _value_number.submit_ui_render_commands(out_commands);
    if (_draw_border)
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(slider_rect,apply_opacity(current_border_color())));
}

void UiSlider::set_slider_config(const UiSliderConfig& config)
{
    apply_slider_config(config);
}

void UiSlider::set_range(float min_value,float max_value)
{
    if (!std::isfinite(min_value))
        min_value = 0.0f;
    if (!std::isfinite(max_value))
        max_value = min_value;
    if (max_value < min_value)
        std::swap(min_value,max_value);

    _min_value = min_value;
    _max_value = max_value;
    (void)set_value_internal(_value,false);
}

void UiSlider::set_value(float value)
{
    (void)set_value_internal(value,true);
}

void UiSlider::set_ratio(float ratio)
{
    if (_max_value <= _min_value)
    {
        set_value(_min_value);
        return;
    }

    const float clamped = std::clamp(ratio,0.0f,1.0f);
    set_value(_min_value + (_max_value - _min_value) * clamped);
}

float UiSlider::min_value() const noexcept
{
    return _min_value;
}

float UiSlider::max_value() const noexcept
{
    return _max_value;
}

float UiSlider::value() const noexcept
{
    return _value;
}

float UiSlider::ratio() const noexcept
{
    return clamped_ratio();
}
void UiSlider::set_step(std::optional<float> step)
{
    if (step && (!std::isfinite(*step) || *step <= 0.0f))
        _step = std::nullopt;
    else
        _step = step;
    (void)set_value_internal(_value,false);
}

const std::optional<float>& UiSlider::step() const noexcept
{
    return _step;
}

void UiSlider::set_label_content(const UiSliderLabelContent& content)
{
    apply_label_content(content);
}

void UiSlider::clear_label_content() noexcept
{
    _text_key.clear();
    _icon = nullptr;
}

void UiSlider::set_text_key(std::string text_key)
{
    clear_label_content();
    _text_key = std::move(text_key);
}

const std::string& UiSlider::text_key() const noexcept
{
    return _text_key;
}

void UiSlider::set_icon_texture(SDL_Texture* texture) noexcept
{
    clear_label_content();
    _icon = texture;
}

void UiSlider::set_label_placement(UiSliderLabelPlacement placement) noexcept
{
    _label_placement = placement;
}

UiSliderLabelPlacement UiSlider::label_placement() const noexcept
{
    return _label_placement;
}

void UiSlider::set_orientation(UiSliderOrientation orientation) noexcept
{
    _orientation = orientation;
}

UiSliderOrientation UiSlider::orientation() const noexcept
{
    return _orientation;
}

void UiSlider::set_value_label_mode(UiSliderValueLabelMode mode) noexcept
{
    _value_label_mode = mode;
}

UiSliderValueLabelMode UiSlider::value_label_mode() const noexcept
{
    return _value_label_mode;
}

void UiSlider::set_sounds(const UiSliderSounds& sounds)
{
    _sounds = sounds;
}

void UiSlider::clear_sounds() noexcept
{
    _sounds.reset();
}

const std::optional<UiSliderSounds>& UiSlider::sounds() const noexcept
{
    return _sounds;
}

void UiSlider::set_on_value_changed(UiSliderValueChangedCallback on_value_changed)
{
    _on_value_changed = std::move(on_value_changed);
}

void UiSlider::set_background_color(elysia::core::Color color) noexcept
{
    _background_color = color;
}

elysia::core::Color UiSlider::background_color() const noexcept
{
    return _background_color;
}

void UiSlider::set_disabled_background_color(elysia::core::Color color) noexcept
{
    _disabled_background_color = color;
}

elysia::core::Color UiSlider::disabled_background_color() const noexcept
{
    return _disabled_background_color;
}

void UiSlider::set_border_color(elysia::core::Color color) noexcept
{
    _border_color = color;
}

elysia::core::Color UiSlider::border_color() const noexcept
{
    return _border_color;
}

void UiSlider::set_disabled_border_color(elysia::core::Color color) noexcept
{
    _disabled_border_color = color;
}

elysia::core::Color UiSlider::disabled_border_color() const noexcept
{
    return _disabled_border_color;
}

void UiSlider::set_fill_color(elysia::core::Color color) noexcept
{
    _fill_color = color;
}

elysia::core::Color UiSlider::fill_color() const noexcept
{
    return _fill_color;
}

void UiSlider::set_disabled_fill_color(elysia::core::Color color) noexcept
{
    _disabled_fill_color = color;
}

elysia::core::Color UiSlider::disabled_fill_color() const noexcept
{
    return _disabled_fill_color;
}

void UiSlider::set_handle_style(const UiSliderHandleStyle& style)
{
    _handle_style = style;
    _handle_style.size = elysia::core::Vector2(
        clamp_non_negative(_handle_style.size.x),
        clamp_non_negative(_handle_style.size.y)
    );
    _handle.set_drag_handle_config(make_drag_handle_config(_handle_style));
}

const UiSliderHandleStyle& UiSlider::handle_style() const noexcept
{
    return _handle_style;
}

void UiSlider::set_text_color(elysia::core::Color color) noexcept
{
    _text_color = color;
}

elysia::core::Color UiSlider::text_color() const noexcept
{
    return _text_color;
}

void UiSlider::set_disabled_text_color(elysia::core::Color color) noexcept
{
    _disabled_text_color = color;
}

elysia::core::Color UiSlider::disabled_text_color() const noexcept
{
    return _disabled_text_color;
}

void UiSlider::set_value_decimal_places(int decimal_places)
{
    _value_number.set_decimal_places(decimal_places);
}

int UiSlider::value_decimal_places() const noexcept
{
    return _value_number.decimal_places();
}
void UiSlider::set_value_trim_trailing_zeros(bool trim_trailing_zeros)
{
    _value_number.set_trim_trailing_zeros(trim_trailing_zeros);
}

bool UiSlider::value_trims_trailing_zeros() const noexcept
{
    return _value_number.trims_trailing_zeros();
}

void UiSlider::set_value_keep_decimal_point(bool keep_decimal_point)
{
    _value_number.set_keep_decimal_point(keep_decimal_point);
}

bool UiSlider::value_keeps_decimal_point() const noexcept
{
    return _value_number.keeps_decimal_point();
}

void UiSlider::set_value_digit_spacing(float spacing)
{
    _value_number.set_digit_spacing(spacing);
}

float UiSlider::value_digit_spacing() const noexcept
{
    return _value_number.digit_spacing();
}

void UiSlider::set_value_fixed_glyph_advance(float advance)
{
    _value_number.set_fixed_glyph_advance(advance);
}

std::optional<float> UiSlider::value_fixed_glyph_advance() const noexcept
{
    return _value_number.fixed_glyph_advance();
}

void UiSlider::clear_value_fixed_glyph_advance()
{
    _value_number.clear_fixed_glyph_advance();
}

void UiSlider::set_value_target_height(float height)
{
    _value_number.set_target_height(height);
}

std::optional<float> UiSlider::value_target_height() const noexcept
{
    return _value_number.target_height();
}

void UiSlider::clear_value_target_height()
{
    _value_number.clear_target_height();
}

void UiSlider::set_bar_thickness(float thickness) noexcept
{
    _bar_thickness = clamp_non_negative(thickness);
}

float UiSlider::bar_thickness() const noexcept
{
    return _bar_thickness;
}

void UiSlider::set_draw_background(bool draw_background) noexcept
{
    _draw_background = draw_background;
}

bool UiSlider::draws_background() const noexcept
{
    return _draw_background;
}

void UiSlider::set_draw_border(bool draw_border) noexcept
{
    _draw_border = draw_border;
}

bool UiSlider::draws_border() const noexcept
{
    return _draw_border;
}

void UiSlider::apply_slider_config(const UiSliderConfig& config)
{
    if (config.slider_sound)
        set_sounds(*config.slider_sound);
    else
        clear_sounds();

    set_label_placement(config.label_placement);
    set_orientation(config.orientation);
    set_value_label_mode(config.value_label_mode);
    set_draw_background(config.draw_background);
    set_draw_border(config.draw_border);
    set_handle_style(config.handle);
    set_bar_thickness(config.bar_thickness);
    set_range(config.min_value,config.max_value);
    set_step(config.step);
    set_value(config.value);
    set_value_decimal_places(config.value_decimal_places);
    set_value_trim_trailing_zeros(config.value_trim_trailing_zeros);
    set_value_keep_decimal_point(config.value_keep_decimal_point);
    set_value_digit_spacing(config.value_digit_spacing);
    if (config.value_fixed_glyph_advance)
        set_value_fixed_glyph_advance(*config.value_fixed_glyph_advance);
    else
        clear_value_fixed_glyph_advance();
    if (config.value_target_height)
        set_value_target_height(*config.value_target_height);
    else
        clear_value_target_height();
    apply_label_content(config.label_content);
}

void UiSlider::initialize_child_widgets()
{
    _bar.set_use_theme(false);
    _bar.set_draw_border(false);
    _bar.set_padding(0);
    _bar.set_fill_direction(BarFillDirection::LeftToRight);
    _handle.set_use_theme(false);
    _label.set_use_theme(false);
    _label.set_draw_background(false);
    _label.set_horizontal_align(TextHorizontalAlign::Center);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_padding(0);
    _value_number.set_use_theme(false);
    _value_number.set_draw_background(false);
    _value_number.set_horizontal_align(TextHorizontalAlign::Center);
    _value_number.set_vertical_align(TextVerticalAlign::Center);
    _value_number.set_padding(0);
    _value_number.set_decimal_places(0);
    _value_number.set_trim_trailing_zeros(true);
    _value_number.set_keep_decimal_point(false);
    _value_number.set_suffix(UiNumberSuffix::None);
    set_handle_style(UiSliderHandleStyle{});
    bind_handle_callbacks();
}

void UiSlider::apply_label_content(const UiSliderLabelContent& content)
{
    std::visit([this](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T,std::monostate>)
            clear_label_content();
        else if constexpr (std::is_same_v<T,UiSliderTextContent>)
            set_text_key(value.text_key);
        else if constexpr (std::is_same_v<T,UiSliderIconContent>)
            set_icon_texture(value.texture);
    },content);
}

void UiSlider::sync_child_rects(const SliderLayout& layout) const
{
    _bar.set_screen_rect(layout.bar_rect);
    _handle.set_screen_rect(layout.handle_rect);
    _handle.set_drag_axis(_orientation == UiSliderOrientation::Horizontal ? UiDragAxis::Horizontal : UiDragAxis::Vertical);
    if (layout.handle_rect.is_empty() || layout.bar_rect.is_empty())
        _handle.clear_drag_bounds();
    else
        _handle.set_drag_bounds(handle_drag_bounds(layout));
    _label.set_screen_rect(layout.label_rect);
    _value_number.set_screen_rect(layout.value_rect);
}

void UiSlider::sync_child_visuals() const
{
    _bar.set_visible(!_bar.screen_rect().is_empty());
    _bar.set_opacity(opacity());
    _bar.set_range(_min_value,_max_value);
    _bar.set_value(_value);
    _bar.set_fill_direction(_orientation == UiSliderOrientation::Horizontal ? BarFillDirection::LeftToRight : BarFillDirection::BottomToTop);
    _bar.set_background_color(current_border_color());
    _bar.set_fill_color(current_fill_color());
    _bar.set_draw_border(false);
    _bar.set_padding(0);

    _handle.set_visible(!_handle.screen_rect().is_empty());
    _handle.set_enabled(is_enabled());
    _handle.set_opacity(opacity());
    _handle.set_focused(is_focused());

    _label.set_visible(!_text_key.empty() && !_icon && !_label.screen_rect().is_empty());
    _label.set_opacity(opacity());
    _label.set_text_key(_text_key);
    _label.set_text_color(current_text_color());
    _label.set_draw_background(false);
    _label.set_horizontal_align(TextHorizontalAlign::Center);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_padding(0);

    _value_number.set_visible(_value_label_mode != UiSliderValueLabelMode::None && !_value_number.screen_rect().is_empty());
    _value_number.set_opacity(opacity());
    _value_number.set_text_color(current_text_color());
    _value_number.set_draw_background(false);
    _value_number.set_horizontal_align(TextHorizontalAlign::Center);
    _value_number.set_vertical_align(TextVerticalAlign::Center);
    _value_number.set_padding(0);
}

void UiSlider::sync_value_number_content() const
{
    _value_number.set_suffix(_value_label_mode == UiSliderValueLabelMode::Percent ? UiNumberSuffix::Percent : UiNumberSuffix::None);
    _value_number.set_value(_value_label_mode == UiSliderValueLabelMode::Percent ? static_cast<double>(clamped_ratio() * 100.0f) : static_cast<double>(_value));
}

UiSlider::SliderLayout UiSlider::compute_layout() const noexcept
{
    SliderLayout layout;
    elysia::core::Rect remaining = screen_rect();
    if (remaining.is_empty())
        return layout;

    const bool has_label = !_text_key.empty() || _icon;
    const elysia::core::Vector2 handle_size(
        clamp_non_negative(_handle_style.size.x),
        clamp_non_negative(_handle_style.size.y)
    );
    const float side_slot_extent = preferred_side_slot_extent(remaining,handle_size);
    const float target_height = _value_number.target_height().value_or(24.0f);
    const float vertical_slot_extent = preferred_vertical_slot_extent(remaining,handle_size,target_height);

    if (_value_label_mode != UiSliderValueLabelMode::None)
    {
        if (_orientation == UiSliderOrientation::Horizontal)
            layout.value_rect = take_right(remaining,side_slot_extent);
        else
            layout.value_rect = take_top(remaining,vertical_slot_extent);
    }

    if (has_label)
    {
        switch (_label_placement)
        {
        case UiSliderLabelPlacement::Left:
            layout.label_rect = take_left(remaining,side_slot_extent);
            break;
        case UiSliderLabelPlacement::Right:
            layout.label_rect = take_right(remaining,side_slot_extent);
            break;
        case UiSliderLabelPlacement::Above:
            layout.label_rect = take_top(remaining,vertical_slot_extent);
            break;
        case UiSliderLabelPlacement::Below:
            layout.label_rect = take_bottom(remaining,vertical_slot_extent);
            break;
        case UiSliderLabelPlacement::Center:
            layout.label_rect = remaining;
            break;
        case UiSliderLabelPlacement::None:
        default:
            break;
        }
    }

    layout.track_area = remaining;
    if (layout.track_area.is_empty())
        return layout;

    if (_orientation == UiSliderOrientation::Horizontal)
    {
        const float thickness = std::min(clamp_non_negative(_bar_thickness),std::max(0.0f,layout.track_area.height()));
        layout.bar_rect = elysia::core::Rect(
            layout.track_area.x(),
            layout.track_area.center().y - thickness * 0.5f,
            layout.track_area.width(),
            thickness
        );
    }
    else
    {
        const float thickness = std::min(clamp_non_negative(_bar_thickness),std::max(0.0f,layout.track_area.width()));
        layout.bar_rect = elysia::core::Rect(
            layout.track_area.center().x - thickness * 0.5f,
            layout.track_area.y(),
            thickness,
            layout.track_area.height()
        );
    }

    if (layout.bar_rect.is_empty())
        return layout;

    const elysia::core::Vector2 clamped_handle_size(
        std::min(handle_size.x,std::max(0.0f,layout.track_area.width())),
        std::min(handle_size.y,std::max(0.0f,layout.track_area.height()))
    );
    if (clamped_handle_size.x <= 0.0f || clamped_handle_size.y <= 0.0f)
        return layout;

    const float ratio = clamped_ratio();
    elysia::core::Vector2 handle_center = layout.bar_rect.center();
    if (_orientation == UiSliderOrientation::Horizontal)
        handle_center.x = layout.bar_rect.x() + layout.bar_rect.width() * ratio;
    else
        handle_center.y = layout.bar_rect.y() + layout.bar_rect.height() * (1.0f - ratio);
    layout.handle_rect = elysia::core::Rect::from_center(handle_center,clamped_handle_size);
    return layout;
}

float UiSlider::clamped_ratio() const noexcept
{
    const float range = _max_value - _min_value;
    if (range <= 0.0f)
        return 0.0f;
    return std::clamp((_value - _min_value) / range,0.0f,1.0f);
}

float UiSlider::snapped_value(float value) const noexcept
{
    const float min_value = std::min(_min_value,_max_value);
    const float max_value = std::max(_min_value,_max_value);
    float clamped = std::clamp(value,min_value,max_value);
    if (!_step || *_step <= 0.0f)
        return clamped;
    if (max_value <= min_value)
        return min_value;

    const float steps = std::round((clamped - min_value) / *_step);
    clamped = min_value + steps * *_step;
    return std::clamp(clamped,min_value,max_value);
}

float UiSlider::action_step() const noexcept
{
    if (_step && *_step > 0.0f)
        return *_step;
    const float range = _max_value - _min_value;
    return range > 0.0f ? range * 0.05f : 0.0f;
}
float UiSlider::ratio_from_point(const SliderLayout& layout,const elysia::core::Vector2& point) const noexcept
{
    if (layout.bar_rect.is_empty())
        return 0.0f;
    if (_orientation == UiSliderOrientation::Horizontal)
    {
        if (layout.bar_rect.width() <= 0.0f)
            return 0.0f;
        return std::clamp((point.x - layout.bar_rect.x()) / layout.bar_rect.width(),0.0f,1.0f);
    }

    if (layout.bar_rect.height() <= 0.0f)
        return 0.0f;
    return std::clamp(1.0f - (point.y - layout.bar_rect.y()) / layout.bar_rect.height(),0.0f,1.0f);
}

elysia::core::Rect UiSlider::handle_drag_bounds(const SliderLayout& layout) const noexcept
{
    if (_orientation == UiSliderOrientation::Horizontal)
    {
        return elysia::core::Rect(
            layout.bar_rect.x() - layout.handle_rect.width() * 0.5f,
            layout.handle_rect.y(),
            layout.bar_rect.width() + layout.handle_rect.width(),
            layout.handle_rect.height()
        );
    }

    return elysia::core::Rect(
        layout.handle_rect.x(),
        layout.bar_rect.y() - layout.handle_rect.height() * 0.5f,
        layout.handle_rect.width(),
        layout.bar_rect.height() + layout.handle_rect.height()
    );
}

bool UiSlider::can_interact() const noexcept
{
    return is_enabled() && is_focused() && is_active() && is_visible();
}

bool UiSlider::can_receive_pointer() const noexcept
{
    return is_enabled() && is_active() && is_visible();
}

bool UiSlider::contains_pointer(int mouse_x,int mouse_y) const noexcept
{
    return screen_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

bool UiSlider::contains_track_or_handle(const SliderLayout& layout,int mouse_x,int mouse_y) const noexcept
{
    const elysia::core::Vector2 point(static_cast<float>(mouse_x),static_cast<float>(mouse_y));
    return layout.bar_rect.contains(point) || layout.handle_rect.contains(point);
}

bool UiSlider::is_primary_pointer_event(const UiInputEvent& event) const noexcept
{
    return event.device == elysia::input::InputDevice::Mouse && event.control == elysia::input::RawInputControl::MouseLeft;
}

bool UiSlider::set_value_internal(float value,bool notify) noexcept
{
    float next_value = std::isfinite(value) ? value : _min_value;
    next_value = snapped_value(next_value);
    if (nearly_equal(next_value,_value))
        return false;

    _value = next_value;
    if (notify && _on_value_changed)
        _on_value_changed(_value);
    return true;
}

bool UiSlider::update_value_from_point(const SliderLayout& layout,const elysia::core::Vector2& point,bool notify) noexcept
{
    if (_max_value <= _min_value)
        return set_value_internal(_min_value,notify);
    return set_value_internal(_min_value + (_max_value - _min_value) * ratio_from_point(layout,point),notify);
}

elysia::core::Rect UiSlider::fitted_texture_rect(const elysia::core::Rect& bounds,SDL_Texture* texture) const noexcept
{
    if (!texture || bounds.is_empty())
        return elysia::core::Rect::zero();

    int texture_width = 0;
    int texture_height = 0;
    if (SDL_QueryTexture(texture,nullptr,nullptr,&texture_width,&texture_height) != 0 || texture_width <= 0 || texture_height <= 0)
        return elysia::core::Rect::zero();

    const float width_scale = bounds.width() / static_cast<float>(texture_width);
    const float height_scale = bounds.height() / static_cast<float>(texture_height);
    const float scale = std::min(1.0f,std::min(width_scale,height_scale));
    if (scale <= 0.0f)
        return elysia::core::Rect::zero();

    const elysia::core::Vector2 render_size(
        static_cast<float>(texture_width) * scale,
        static_cast<float>(texture_height) * scale
    );
    return elysia::core::Rect::from_center(bounds.center(),render_size);
}

elysia::core::Color UiSlider::current_background_color() const noexcept
{
    return is_enabled() ? _background_color : _disabled_background_color;
}

elysia::core::Color UiSlider::current_border_color() const noexcept
{
    return is_enabled() ? _border_color : _disabled_border_color;
}

elysia::core::Color UiSlider::current_fill_color() const noexcept
{
    return is_enabled() ? _fill_color : _disabled_fill_color;
}

elysia::core::Color UiSlider::current_text_color() const noexcept
{
    return is_enabled() ? _text_color : _disabled_text_color;
}
void UiSlider::bind_handle_callbacks()
{
    _handle.set_on_dragged([this](const elysia::core::Vector2& center)
    {
        const SliderLayout layout = compute_layout();
        const bool changed = update_value_from_point(layout,center,true);
        _drag_value_changed = changed || _drag_value_changed;
        if (changed)
            play_slide_sound_if_allowed();
    });
    _handle.set_on_drag_ended([this](const elysia::core::Vector2&)
    {
        const bool should_settle = _drag_value_changed;
        _drag_value_changed = false;
        if (should_settle && _sounds)
            play_sound_if_set(_sounds->on_settle);
    });
}

void UiSlider::clear_drag_state() noexcept
{
    _handle.cancel_drag();
    _drag_value_changed = false;
}

void UiSlider::play_sound_if_set(std::string_view sound_key) const
{
    if (sound_key.empty())
        return;
    elysia::audio::AudioService::instance()->play_sound(sound_key);
}

void UiSlider::play_slide_sound_if_allowed()
{
    if (!_sounds)
        return;
    if (_sounds->on_slide.empty())
        return;

    const std::uint32_t now = static_cast<std::uint32_t>(SDL_GetTicks());
    const std::uint32_t min_interval_ms = static_cast<std::uint32_t>(std::max(0.0,_sounds->min_slide_sound_interval) * 1000.0);
    if (!_has_last_slide_sound_tick || now - _last_slide_sound_ticks >= min_interval_ms)
    {
        play_sound_if_set(_sounds->on_slide);
        _last_slide_sound_ticks = now;
        _has_last_slide_sound_tick = true;
    }
}
}
