#include "gamepad_input_translator.h"

#include <SDL.h>

std::vector<RawInputEvent> GamepadInputTranslator::translate_event(const SDL_Event& event) const
{
    std::vector<RawInputEvent> events;

    switch (event.type)
    {
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        append_controller_button_events(
            events,
            event.cbutton.button,
            input_event_type(event.type == SDL_CONTROLLERBUTTONDOWN)
        );
        break;

    case SDL_CONTROLLERAXISMOTION:
    {
        RawInputEvent input_event;
        input_event.type = RawInputEventType::AxisChanged;
        input_event.device = InputDevice::Gamepad;
        input_event.axis_value = static_cast<float>(event.caxis.value) / 32767.0f;

        switch (event.caxis.axis)
        {
        case SDL_CONTROLLER_AXIS_LEFTX:
            input_event.axis = RawInputAxis::GamepadLeftX;
            events.push_back(input_event);
            break;

        case SDL_CONTROLLER_AXIS_LEFTY:
            input_event.axis = RawInputAxis::GamepadLeftY;
            events.push_back(input_event);
            break;

        default:
            break;
        }

        break;
    }

    default:
        break;
    }

    return events;
}

void GamepadInputTranslator::append_controller_button_events(
    std::vector<RawInputEvent>& events,
    Uint8 button,
    RawInputEventType type
) const
{
    switch (static_cast<SDL_GameControllerButton>(button))
    {
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        append_event(events, RawInputControl::GamepadDPadLeft, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        append_event(events, RawInputControl::GamepadDPadRight, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        append_event(events, RawInputControl::GamepadDPadUp, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        append_event(events, RawInputControl::GamepadDPadDown, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_A:
        append_event(events, RawInputControl::GamepadSouth, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_B:
        append_event(events, RawInputControl::GamepadEast, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_X:
        append_event(events, RawInputControl::GamepadWest, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        append_event(events, RawInputControl::GamepadNorth, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        append_event(events, RawInputControl::GamepadLeftShoulder, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        append_event(events, RawInputControl::GamepadRightShoulder, type, InputDevice::Gamepad);
        break;
    case SDL_CONTROLLER_BUTTON_START:
        append_event(events, RawInputControl::GamepadStart, type, InputDevice::Gamepad);
        break;
    default:
        break;
    }
}
