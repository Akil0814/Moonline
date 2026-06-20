#include "application_scene.h"

#include "../../engine/scene/scene_input_order.h"

#include <algorithm>

namespace
{
template <typename Entry>
void erase_destroyed_entries(std::vector<Entry>& entries)
{
    std::erase_if(entries, [](const Entry& entry)
    {
        return !entry.object || entry.object->is_destroyed();
    });
}
}

void ApplicationScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events
)
{
    elysia::scene::Scene::on_input(input, events);
    prune_domain_receivers();

    const elysia::input::InputContext context = input_context();

    switch (context)
    {
    case elysia::input::InputContext::UI:
    {
        const elysia::ui::UiInputFrame ui_frame = _ui_input_router.route_frame(input);
        dispatch_ui_frame(ui_frame);

        for (const elysia::input::RawInputEvent& raw_event : events)
        {
            dispatch_ui_events(_ui_input_router.route_event(raw_event));
        }

        dispatch_ui_events(_ui_input_router.synthesize_events(input));
        break;
    }

    case elysia::input::InputContext::Gameplay:
    {
        _ui_input_router.reset_transient_state();

        const GameplayInputFrame gameplay_frame = _gameplay_input_router.route_frame(input);
        dispatch_gameplay_frame(gameplay_frame);

        for (const elysia::input::RawInputEvent& raw_event : events)
        {
            dispatch_gameplay_events(_gameplay_input_router.route_event(raw_event));
        }

        break;
    }

    case elysia::input::InputContext::Dialogue:
    case elysia::input::InputContext::Debug:
    case elysia::input::InputContext::None:
    default:
        _ui_input_router.reset_transient_state();
        break;
    }
}

elysia::input::InputContext ApplicationScene::input_context() const
{
    return elysia::input::InputContext::None;
}

void ApplicationScene::on_scene_object_registered(elysia::core::SceneObject& object)
{
    elysia::scene::Scene::on_scene_object_registered(object);

    if (auto* receiver = dynamic_cast<elysia::ui::UiInputFrameReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _ui_frame_receivers,
            UiInputFrameReceiverEntry{ &object, receiver }
        );
    }

    if (auto* receiver = dynamic_cast<elysia::ui::UiInputEventReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _ui_event_receivers,
            UiInputEventReceiverEntry{ &object, receiver }
        );
    }

    if (auto* receiver = dynamic_cast<GameplayInputFrameReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _gameplay_frame_receivers,
            GameplayInputFrameReceiverEntry{ &object, receiver }
        );
    }

    if (auto* receiver = dynamic_cast<GameplayInputEventReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _gameplay_event_receivers,
            GameplayInputEventReceiverEntry{ &object, receiver }
        );
    }
}

void ApplicationScene::prune_domain_receivers()
{
    erase_destroyed_entries(_ui_frame_receivers);
    erase_destroyed_entries(_ui_event_receivers);
    erase_destroyed_entries(_gameplay_frame_receivers);
    erase_destroyed_entries(_gameplay_event_receivers);
}

void ApplicationScene::dispatch_ui_frame(const elysia::ui::UiInputFrame& input)
{
    for (const UiInputFrameReceiverEntry& entry : _ui_frame_receivers)
    {
        elysia::core::SceneObject* object = entry.object;

        if (!object || object->is_destroyed() || !object->is_active())
        {
            continue;
        }

        if (_paused && !object->receive_input_when_paused())
        {
            continue;
        }

        entry.receiver->on_ui_input_frame(input);
    }
}

void ApplicationScene::dispatch_ui_events(const std::vector<elysia::ui::UiInputEvent>& events)
{
    for (const elysia::ui::UiInputEvent& ui_event : events)
    {
        for (const UiInputEventReceiverEntry& entry : _ui_event_receivers)
        {
            elysia::core::SceneObject* object = entry.object;

            if (!object || object->is_destroyed() || !object->is_active())
            {
                continue;
            }

            if (_paused && !object->receive_input_when_paused())
            {
                continue;
            }

            if (entry.receiver->on_ui_input_event(ui_event))
            {
                break;
            }
        }
    }
}

void ApplicationScene::dispatch_gameplay_frame(const GameplayInputFrame& input)
{
    for (const GameplayInputFrameReceiverEntry& entry : _gameplay_frame_receivers)
    {
        elysia::core::SceneObject* object = entry.object;

        if (!object || object->is_destroyed() || !object->is_active())
        {
            continue;
        }

        if (_paused && !object->receive_input_when_paused())
        {
            continue;
        }

        entry.receiver->on_gameplay_input_frame(input);
    }
}

void ApplicationScene::dispatch_gameplay_events(const std::vector<GameplayInputEvent>& events)
{
    for (const GameplayInputEvent& gameplay_event : events)
    {
        for (const GameplayInputEventReceiverEntry& entry : _gameplay_event_receivers)
        {
            elysia::core::SceneObject* object = entry.object;

            if (!object || object->is_destroyed() || !object->is_active())
            {
                continue;
            }

            if (_paused && !object->receive_input_when_paused())
            {
                continue;
            }

            if (entry.receiver->on_gameplay_input_event(gameplay_event))
            {
                break;
            }
        }
    }
}
