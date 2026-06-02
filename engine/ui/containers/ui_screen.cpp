#include "ui_screen.h"

#include "../ui_settings.h"
#include "../ui_mouse_utils.h"

#include <algorithm>

UiScreen::UiScreen(Vector2 position, Vector2 size, int order)
    : UiPanel(position, size, order)
{
    set_panel_theme_role(UiPanelThemeRole::Screen);
    UiTransitionState shown_state;
    shown_state._transform = transform();
    shown_state._background_alpha = background_alpha();

    UiTransitionState hidden_state = shown_state;
    hidden_state._transform.translation.y += 24.0f;
    hidden_state._transform.scale = { 0.96f, 0.96f };
    hidden_state._background_alpha = 0;

    _transition.set_hidden_state(hidden_state);
    _transition.set_shown_state(shown_state);
    _transition.jump_to_shown();
}

void UiScreen::on_update(double delta)
{
    if (!_is_open && !_is_closing)
    {
        return;
    }

    if (!is_transition_enabled() && _transition.is_playing())
    {
        if (_is_open)
        {
            _transition.jump_to_shown();
        }
        else
        {
            _transition.jump_to_hidden();
            set_visible(false);
            set_active(false);
            _is_closing = false;
        }

        apply_transition_state();
    }

    if (is_transition_enabled() && _transition.is_playing())
    {
        _transition.update(delta);
        apply_transition_state();

        if (!_transition.is_playing() && _is_closing)
        {
            _is_closing = false;
            set_visible(false);
            set_active(false);
            apply_control_focus();
            return;
        }
    }

    UiPanel::on_update(delta);
}

void UiScreen::on_render(SDL_Renderer* renderer)
{
    if (!_is_open && !_is_closing)
    {
        return;
    }

    UiPanel::on_render(renderer);
}

void UiScreen::on_input(const InputSnapshot& input)
{
    if (!_is_open || _is_closing || !_input_enabled)
    {
        return;
    }

    cleanup_focusable_controls();

    if (!ui_mouse_input_allowed(input))
    {
        _was_mouse_down = false;
        UiPanel::on_input(input);
        return;
    }

    const UiMouseState mouse_state = ui_current_mouse_state();

    if (ui_left_mouse_pressed(mouse_state, _was_mouse_down))
    {
        for (int index = static_cast<int>(_focusable_controls.size()) - 1; index >= 0; --index)
        {
            std::shared_ptr<UiFocusable> control = _focusable_controls[static_cast<size_t>(index)].lock();
            if (!control || !control->is_enabled())
            {
                continue;
            }

            const GameObject* object = control->game_object();
            if (!object)
            {
                continue;
            }

            if (ui_contains_point(object->rect(), mouse_state._position))
            {
                _focused_control_index = index;
                apply_control_focus();
                break;
            }
        }
    }

    _was_mouse_down = mouse_state._left_button_down;
    UiPanel::on_input(input);
}

void UiScreen::on_input_event(const InputEvent& event)
{
    if (!_is_open || _is_closing || !_input_enabled)
    {
        return;
    }

    std::shared_ptr<UiFocusable> control = focused_control();
    if (control && control->handle_focused_input_event(event))
    {
        return;
    }

    if (event.type == InputEventType::Pressed && is_focus_navigation_event(event))
    {
        switch (event.action)
        {
        case InputAction::Left:
        case InputAction::Up:
            focus_previous_control();
            return;

        case InputAction::Right:
        case InputAction::Down:
        case InputAction::Tab:
            focus_next_control();
            return;

        default:
            break;
        }
    }
}

void UiScreen::reset()
{
    UiPanel::reset();
    set_panel_theme_role(UiPanelThemeRole::Screen);
    clear_children();
    set_transform(UiLayoutTransform{});
    _is_open = true;
    _is_closing = false;
    _input_enabled = true;
    _transition_enabled = false;
    _was_mouse_down = false;
    _focused_control_index = -1;
    _focusable_controls.clear();
    UiTransitionState shown_state;
    shown_state._transform = transform();
    shown_state._background_alpha = background_alpha();
    UiTransitionState hidden_state = shown_state;
    hidden_state._transform.translation.y += 24.0f;
    hidden_state._transform.scale = { 0.96f, 0.96f };
    hidden_state._background_alpha = 0;
    _transition.set_hidden_state(hidden_state);
    _transition.set_shown_state(shown_state);
    _transition.jump_to_shown();
}

void UiScreen::open()
{
    _is_open = true;
    _is_closing = false;
    set_visible(true);
    set_active(true);
    if (is_transition_enabled())
    {
        _transition.play_forward();
        apply_transition_state();
    }
    else
    {
        _transition.jump_to_shown();
        apply_transition_state();
    }

    if (!_transition.is_playing())
    {
        _is_closing = false;
    }

    apply_control_focus();
}

void UiScreen::close()
{
    if (is_transition_enabled())
    {
        _is_open = false;
        _is_closing = true;
        set_visible(true);
        set_active(true);
        _transition.play_backward();
        apply_transition_state();
    }
    else
    {
        _is_open = false;
        _is_closing = false;
        set_visible(false);
        set_active(false);
        _transition.jump_to_hidden();
        apply_transition_state();
    }

    if (!_transition.is_playing() && !_is_open)
    {
        _is_closing = false;
        set_visible(false);
        set_active(false);
    }

    apply_control_focus();
}

void UiScreen::set_open(bool open)
{
    if (open)
    {
        UiScreen::open();
        return;
    }

    UiScreen::close();
}

bool UiScreen::is_open() const
{
    return _is_open;
}

void UiScreen::set_input_enabled(bool enabled)
{
    _input_enabled = enabled;
    apply_control_focus();
}

bool UiScreen::is_input_enabled() const
{
    return _input_enabled;
}

void UiScreen::set_transition_enabled(bool enabled)
{
    _transition_enabled = enabled;
    if (!_transition_enabled)
    {
        if (_is_open)
        {
            _transition.jump_to_shown();
        }
        else
        {
            _transition.jump_to_hidden();
        }

        apply_transition_state();
    }
}

bool UiScreen::is_transition_enabled() const
{
    return _transition_enabled && UiSettings::are_animations_enabled();
}

void UiScreen::configure_transition(
    const UiTransitionState& hidden_state,
    const UiTransitionState& shown_state,
    double duration_seconds,
    UiEasing easing
)
{
    _transition.set_hidden_state(hidden_state);
    _transition.set_shown_state(shown_state);
    _transition.set_duration_seconds(duration_seconds);
    _transition.set_easing(easing);

    if (_is_open)
    {
        _transition.jump_to_shown();
    }
    else
    {
        _transition.jump_to_hidden();
    }

    apply_transition_state();
}

void UiScreen::set_transition_duration(double duration_seconds)
{
    _transition.set_duration_seconds(duration_seconds);
}

void UiScreen::set_transition_easing(UiEasing easing)
{
    _transition.set_easing(easing);
}

const UiTransition& UiScreen::transition() const
{
    return _transition;
}

void UiScreen::register_focusable_control(const std::shared_ptr<UiFocusable>& control)
{
    if (!control)
    {
        return;
    }

    _focusable_controls.push_back(control);
    cleanup_focusable_controls();

    if (_focused_control_index < 0)
    {
        _focused_control_index = 0;
    }

    apply_control_focus();
}

void UiScreen::clear_focusable_controls()
{
    for (std::weak_ptr<UiFocusable>& control_handle : _focusable_controls)
    {
        std::shared_ptr<UiFocusable> control = control_handle.lock();
        if (control)
        {
            control->set_focused(false);
        }
    }

    _focusable_controls.clear();
    _focused_control_index = -1;
}

void UiScreen::focus_next_control()
{
    cleanup_focusable_controls();
    if (_focusable_controls.empty())
    {
        return;
    }

    if (_focused_control_index < 0)
    {
        _focused_control_index = 0;
    }
    else
    {
        _focused_control_index = (_focused_control_index + 1)
            % static_cast<int>(_focusable_controls.size());
    }

    apply_control_focus();
}

void UiScreen::focus_previous_control()
{
    cleanup_focusable_controls();
    if (_focusable_controls.empty())
    {
        return;
    }

    if (_focused_control_index < 0)
    {
        _focused_control_index = 0;
    }
    else
    {
        _focused_control_index -= 1;
        if (_focused_control_index < 0)
        {
            _focused_control_index = static_cast<int>(_focusable_controls.size()) - 1;
        }
    }

    apply_control_focus();
}

void UiScreen::set_focused_control(int index)
{
    cleanup_focusable_controls();
    if (_focusable_controls.empty())
    {
        _focused_control_index = -1;
        return;
    }

    if (index < 0)
    {
        _focused_control_index = -1;
    }
    else
    {
        const int max_index = static_cast<int>(_focusable_controls.size()) - 1;
        _focused_control_index = std::min(index, max_index);
    }

    apply_control_focus();
}

int UiScreen::focused_control_index() const
{
    return _focused_control_index;
}

void UiScreen::apply_transition_state()
{
    set_transform(_transition.current_state()._transform);
    set_background_alpha(_transition.current_state()._background_alpha);
}

void UiScreen::cleanup_focusable_controls()
{
    _focusable_controls.erase(
        std::remove_if(_focusable_controls.begin(), _focusable_controls.end(),
            [](const std::weak_ptr<UiFocusable>& control)
            {
                return control.expired();
            }),
        _focusable_controls.end()
    );

    if (_focusable_controls.empty())
    {
        _focused_control_index = -1;
        return;
    }

    const int max_index = static_cast<int>(_focusable_controls.size()) - 1;
    if (_focused_control_index > max_index)
    {
        _focused_control_index = max_index;
    }
}

void UiScreen::apply_control_focus()
{
    cleanup_focusable_controls();

    for (int index = 0; index < static_cast<int>(_focusable_controls.size()); ++index)
    {
        std::shared_ptr<UiFocusable> control = _focusable_controls[static_cast<size_t>(index)].lock();
        if (!control)
        {
            continue;
        }

        control->set_focused(
            _is_open
            && _input_enabled
            && control->is_enabled()
            && index == _focused_control_index
        );
    }
}

std::shared_ptr<UiFocusable> UiScreen::focused_control() const
{
    if (_focused_control_index < 0 || _focused_control_index >= static_cast<int>(_focusable_controls.size()))
    {
        return nullptr;
    }

    return _focusable_controls[static_cast<size_t>(_focused_control_index)].lock();
}

bool UiScreen::is_focus_navigation_event(const InputEvent& event) const
{
    if (event.type != InputEventType::Pressed)
    {
        return false;
    }

    if (event.action == InputAction::Tab)
    {
        return true;
    }

    if (direction() == UiLayoutDirection::Horizontal)
    {
        return event.action == InputAction::Left || event.action == InputAction::Right;
    }

    return event.action == InputAction::Up || event.action == InputAction::Down;
}
