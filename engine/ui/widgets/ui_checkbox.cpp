#include "ui_checkbox.h"

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
[[nodiscard]] bool has_visual_state_textures(const UiCheckboxVisualStateTextures& textures) noexcept
{
    return textures.idle
        && textures.focused
        && textures.pushed
        && textures.disabled;
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

UiCheckbox::UiCheckbox(const elysia::core::Rect& rect,int order) noexcept
    : UiControl(rect,order) {}

UiCheckbox::UiCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiCheckbox(elysia::core::Rect(position.x,position.y,size.x,size.y),order) {}

UiCheckbox::UiCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiCheckbox(elysia::core::Rect::from_center(center,size),order) {}

UiCheckbox::UiCheckbox(const elysia::core::Rect& rect,const UiCheckboxConfig& config,int order) noexcept
    : UiCheckbox(rect,order)
{
    set_checkbox_config(config);
}

UiCheckbox::UiCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiCheckboxConfig& config,int order) noexcept
    : UiCheckbox(elysia::core::Rect(position.x,position.y,size.x,size.y),config,order) {}

UiCheckbox::UiCheckbox(
    const elysia::core::Vector2& center,const elysia::core::Vector2& size,
    UiFromCenterTag,const UiCheckboxConfig& config,int order
) noexcept : UiCheckbox(elysia::core::Rect::from_center(center,size),config,order) {}

void UiCheckbox::reset() noexcept
{
    UiControl::reset();
    set_use_theme(false);

    _textures.reset();
    _sounds.reset();
    _on_toggled = nullptr;
    _state = UiCheckboxState::Unchecked;
    _style = UiStyleDefaults::checkbox();
    _padding = 4;
    _is_pushed = false;
}

void UiCheckbox::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    if (!enabled)
        clear_pushed_state();
}

void UiCheckbox::set_focused(bool focused)
{
    const bool was_focused = is_focused();
    UiControl::set_focused(focused);
    if (!is_focused())
        clear_pushed_state();
    if (!was_focused && is_focused() && _sounds)
        play_sound_if_set(_sounds->focus);
}

bool UiCheckbox::on_ui_input_event(const UiInputEvent& event)
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
            return toggle_internal(true,true);

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
        const bool should_toggle = _is_pushed;
        clear_pushed_state();
        if (should_toggle)
            return toggle_internal(true,true);
        return false;
    }

    return false;
}

void UiCheckbox::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect rect = checkbox_rect();
    if (rect.is_empty())
        return;

    if (uses_texture_rendering())
    {
        SDL_Texture* texture = current_state_texture();
        if (!texture)
            return;

        elysia::core::UiRenderCommand command = elysia::core::make_ui_texture_command(texture,rect);
        apply_opacity(command);
        out_commands.push_back(command);
        return;
    }

    if (_style.chrome.draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(current_background_color())));
    if (_style.chrome.draw_border)
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(current_border_color())));

    const elysia::core::Color mark_color = apply_opacity(current_checkmark_color());
    if (_style.mark_style == UiCheckboxMarkStyle::RadioDot)
    {
        if (_style.chrome.draw_background)
            submit_fill_disc(out_commands,rect,apply_opacity(current_background_color()));
        if (_style.chrome.draw_border)
            submit_fill_disc(out_commands,rect,apply_opacity(current_border_color()));

        const float inset = std::max(2.0f,std::min(rect.width(),rect.height()) * 0.14f);
        const elysia::core::Rect inner_rect(
            rect.x() + inset,
            rect.y() + inset,
            std::max(0.0f,rect.width() - inset * 2.0f),
            std::max(0.0f,rect.height() - inset * 2.0f)
        );
        if (_style.chrome.draw_background && !inner_rect.is_empty())
            submit_fill_disc(out_commands,inner_rect,apply_opacity(current_background_color()));

        if (_state == UiCheckboxState::Checked)
        {
            const float dot_inset = std::max(4.0f,std::min(rect.width(),rect.height()) * 0.32f);
            const elysia::core::Rect dot_rect(
                rect.x() + dot_inset,
                rect.y() + dot_inset,
                std::max(0.0f,rect.width() - dot_inset * 2.0f),
                std::max(0.0f,rect.height() - dot_inset * 2.0f)
            );
            if (!dot_rect.is_empty())
                submit_fill_disc(out_commands,dot_rect,mark_color);
        }
        return;
    }

    if (_state == UiCheckboxState::Checked)
    {
        if (_style.mark_style == UiCheckboxMarkStyle::FilledBox)
        {
            const float inset = std::max(2.0f,std::min(rect.width(),rect.height()) * 0.22f);
            const elysia::core::Rect fill_rect(
                rect.x() + inset,
                rect.y() + inset,
                std::max(0.0f,rect.width() - inset * 2.0f),
                std::max(0.0f,rect.height() - inset * 2.0f)
            );
            if (!fill_rect.is_empty())
                out_commands.push_back(elysia::core::make_ui_fill_rect_command(fill_rect,mark_color));
        }
        else
        {
            const elysia::core::Vector2 start(rect.x() + rect.width() * 0.22f,rect.y() + rect.height() * 0.54f);
            const elysia::core::Vector2 mid(rect.x() + rect.width() * 0.43f,rect.y() + rect.height() * 0.76f);
            const elysia::core::Vector2 end(rect.x() + rect.width() * 0.80f,rect.y() + rect.height() * 0.28f);
            out_commands.push_back(elysia::core::make_ui_draw_line_command(start,mid,mark_color));
            out_commands.push_back(elysia::core::make_ui_draw_line_command(mid,end,mark_color));
        }
    }
    else if (_state == UiCheckboxState::Indeterminate)
    {
        const elysia::core::Vector2 start(rect.x() + rect.width() * 0.22f,rect.center().y);
        const elysia::core::Vector2 end(rect.x() + rect.width() * 0.78f,rect.center().y);
        out_commands.push_back(elysia::core::make_ui_draw_line_command(start,end,mark_color));
    }
}

void UiCheckbox::set_checkbox_config(const UiCheckboxConfig& config)
{
    apply_checkbox_config(config);
}

void UiCheckbox::set_state(UiCheckboxState state) noexcept
{
    (void)set_state_internal(state,false);
}

UiCheckboxState UiCheckbox::state() const noexcept
{
    return _state;
}

void UiCheckbox::set_checked(bool checked) noexcept
{
    (void)set_state_internal(checked ? UiCheckboxState::Checked : UiCheckboxState::Unchecked,false);
}

bool UiCheckbox::is_checked() const noexcept
{
    return _state == UiCheckboxState::Checked;
}

bool UiCheckbox::is_indeterminate() const noexcept
{
    return _state == UiCheckboxState::Indeterminate;
}

void UiCheckbox::toggle()
{
    (void)toggle_internal(true,false);
}

void UiCheckbox::set_style(const UiCheckboxStyle& style) noexcept
{
    _style = style;
}

const UiCheckboxStyle& UiCheckbox::style() const noexcept
{
    return _style;
}

void UiCheckbox::set_state_textures(const UiCheckboxTextures& textures)
{
    _textures = textures;
}

void UiCheckbox::clear_state_textures() noexcept
{
    _textures.reset();
}

const std::optional<UiCheckboxTextures>& UiCheckbox::state_textures() const noexcept
{
    return _textures;
}

bool UiCheckbox::has_complete_state_textures() const noexcept
{
    return _textures
        && has_visual_state_textures(_textures->unchecked)
        && has_visual_state_textures(_textures->checked)
        && has_visual_state_textures(_textures->indeterminate);
}

void UiCheckbox::set_sounds(const UiCheckboxSounds& sounds)
{
    _sounds = sounds;
}

void UiCheckbox::clear_sounds() noexcept
{
    _sounds.reset();
}

const std::optional<UiCheckboxSounds>& UiCheckbox::sounds() const noexcept
{
    return _sounds;
}

void UiCheckbox::set_on_toggled(UiCheckboxToggledCallback on_toggled)
{
    _on_toggled = std::move(on_toggled);
}

void UiCheckbox::set_padding(int padding) noexcept
{
    _padding = std::max(0,padding);
}

int UiCheckbox::padding() const noexcept
{
    return _padding;
}

void UiCheckbox::apply_checkbox_config(const UiCheckboxConfig& config)
{
    if (config.textures)
        set_state_textures(*config.textures);
    else
        clear_state_textures();

    if (config.sounds)
        set_sounds(*config.sounds);
    else
        clear_sounds();

    if (config.style)
        set_style(*config.style);
}

bool UiCheckbox::set_state_internal(UiCheckboxState state,bool notify) noexcept
{
    if (_state == state)
        return false;

    _state = state;
    if (notify && _on_toggled)
        _on_toggled(_state);
    return true;
}

bool UiCheckbox::toggle_internal(bool notify,bool play_toggle_sound) noexcept
{
    const bool changed = set_state_internal(toggled_state(_state),notify);
    if (changed && play_toggle_sound && _sounds)
        play_sound_if_set(_sounds->toggle);
    return changed;
}

bool UiCheckbox::can_interact() const noexcept
{
    return is_enabled() && is_focused() && is_active() && is_visible();
}

bool UiCheckbox::can_receive_pointer() const noexcept
{
    return is_enabled() && is_active() && is_visible();
}

bool UiCheckbox::contains_pointer(int mouse_x,int mouse_y) const noexcept
{
    return screen_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

bool UiCheckbox::is_primary_pointer_event(const UiInputEvent& event) const noexcept
{
    return event.device == elysia::input::InputDevice::Mouse
        && event.control == elysia::input::RawInputControl::MouseLeft;
}

void UiCheckbox::clear_pushed_state() noexcept
{
    _is_pushed = false;
}

void UiCheckbox::play_sound_if_set(const std::string& sound_key) const
{
    if (sound_key.empty())
        return;
    elysia::audio::AudioService::instance()->play_sound(sound_key);
}

elysia::core::Rect UiCheckbox::content_rect() const noexcept
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

elysia::core::Rect UiCheckbox::checkbox_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    if (content.is_empty())
        return elysia::core::Rect::zero();

    const float side = std::min(content.width(),content.height());
    if (side <= 0.0f)
        return elysia::core::Rect::zero();

    return elysia::core::Rect::from_center(content.center(),elysia::core::Vector2(side,side));
}

const UiCheckboxVisualStateTextures* UiCheckbox::current_state_textures() const noexcept
{
    if (!_textures)
        return nullptr;

    switch (_state)
    {
    case UiCheckboxState::Checked:
        return &_textures->checked;
    case UiCheckboxState::Indeterminate:
        return &_textures->indeterminate;
    case UiCheckboxState::Unchecked:
    default:
        return &_textures->unchecked;
    }
}

SDL_Texture* UiCheckbox::current_state_texture() const noexcept
{
    const UiCheckboxVisualStateTextures* textures = current_state_textures();
    if (!textures)
        return nullptr;
    if (!is_enabled())
        return textures->disabled;
    if (_is_pushed)
        return textures->pushed;
    if (is_focused())
        return textures->focused;
    return textures->idle;
}

bool UiCheckbox::uses_texture_rendering() const noexcept
{
    return has_complete_state_textures();
}

elysia::core::Color UiCheckbox::current_background_color() const noexcept
{
    return resolve_interactive_color(_style.chrome.background,is_enabled(),is_focused(),_is_pushed);
}

elysia::core::Color UiCheckbox::current_border_color() const noexcept
{
    return resolve_enabled_disabled_color(_style.chrome.border,is_enabled());
}

elysia::core::Color UiCheckbox::current_checkmark_color() const noexcept
{
    return resolve_enabled_disabled_color(_style.mark,is_enabled());
}

UiCheckboxState UiCheckbox::toggled_state(UiCheckboxState state) noexcept
{
    switch (state)
    {
    case UiCheckboxState::Checked:
        return UiCheckboxState::Unchecked;
    case UiCheckboxState::Indeterminate:
        return UiCheckboxState::Checked;
    case UiCheckboxState::Unchecked:
    default:
        return UiCheckboxState::Checked;
    }
}
}

