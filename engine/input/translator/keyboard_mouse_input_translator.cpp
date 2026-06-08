#include "keyboard_mouse_input_translator.h"

#include <SDL.h>

std::vector<RawInputEvent> KeyboardMouseInputTranslator::translate_event(const SDL_Event& event) const
{
    std::vector<RawInputEvent> events;

    switch (event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event.type == SDL_KEYDOWN && event.key.repeat != 0)
        {
            return events;
        }

        const std::optional<RawInputControl> control = control_from_key(event.key.keysym.sym);
        if (control)
        {
            append_event(
                events,
                *control,
                input_event_type(event.type == SDL_KEYDOWN),
                InputDevice::Keyboard
            );
        }

        break;
    }

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
        const std::optional<RawInputControl> control = control_from_mouse_button(event.button.button);
        if (control)
        {
            append_event(
                events,
                *control,
                input_event_type(event.type == SDL_MOUSEBUTTONDOWN),
                InputDevice::Mouse
            );
        }

        break;
    }

    case SDL_MOUSEWHEEL:
    {
        RawInputEvent input_event;
        input_event.type = RawInputEventType::MouseWheel;
        input_event.device = InputDevice::Mouse;
        input_event.wheel_x = event.wheel.x;
        input_event.wheel_y = event.wheel.y;
        events.push_back(input_event);
        break;
    }

    case SDL_TEXTINPUT:
    {
        RawInputEvent input_event;
        input_event.type = RawInputEventType::TextInput;
        input_event.device = InputDevice::Keyboard;
        input_event.text = event.text.text;
        events.push_back(input_event);
        break;
    }

    case SDL_TEXTEDITING:
    {
        RawInputEvent input_event;
        input_event.type = RawInputEventType::TextEditing;
        input_event.device = InputDevice::Keyboard;
        input_event.text = event.edit.text;
        events.push_back(input_event);
        break;
    }

    default:
        break;
    }

    return events;
}

std::optional<RawInputControl> KeyboardMouseInputTranslator::control_from_key(SDL_Keycode key) const
{
    switch (key)
    {
    case SDLK_a:
        return RawInputControl::KeyA;
    case SDLK_d:
        return RawInputControl::KeyD;
    case SDLK_w:
        return RawInputControl::KeyW;
    case SDLK_s:
        return RawInputControl::KeyS;
    case SDLK_LEFT:
        return RawInputControl::KeyLeft;
    case SDLK_RIGHT:
        return RawInputControl::KeyRight;
    case SDLK_UP:
        return RawInputControl::KeyUp;
    case SDLK_DOWN:
        return RawInputControl::KeyDown;
    case SDLK_RETURN:
        return RawInputControl::KeyEnter;
    case SDLK_KP_ENTER:
        return RawInputControl::KeyNumpadEnter;
    case SDLK_ESCAPE:
        return RawInputControl::KeyEscape;
    case SDLK_p:
        return RawInputControl::KeyP;
    case SDLK_SPACE:
        return RawInputControl::KeySpace;
    case SDLK_j:
        return RawInputControl::KeyJ;
    case SDLK_k:
        return RawInputControl::KeyK;
    case SDLK_l:
        return RawInputControl::KeyL;
    case SDLK_LSHIFT:
        return RawInputControl::KeyLeftShift;
    case SDLK_RSHIFT:
        return RawInputControl::KeyRightShift;
    case SDLK_BACKSPACE:
        return RawInputControl::KeyBackspace;
    case SDLK_DELETE:
        return RawInputControl::KeyDelete;
    case SDLK_HOME:
        return RawInputControl::KeyHome;
    case SDLK_END:
        return RawInputControl::KeyEnd;
    case SDLK_TAB:
        return RawInputControl::KeyTab;
    default:
        return std::nullopt;
    }
}

std::optional<RawInputControl> KeyboardMouseInputTranslator::control_from_mouse_button(Uint8 button) const
{
    switch (button)
    {
    case SDL_BUTTON_LEFT:
        return RawInputControl::MouseLeft;
    case SDL_BUTTON_RIGHT:
        return RawInputControl::MouseRight;
    default:
        return std::nullopt;
    }
}
