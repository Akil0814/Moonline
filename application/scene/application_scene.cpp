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
    const std::vector<elysia::input::RawInputEvent>& events)
{
    elysia::scene::Scene::on_input(input, events);
    prune_domain_receivers();

    if (!accepts_gameplay_input())
    {
        return;
    }

    const arcneco::input::GameplayInputFrame gameplay_frame = _gameplay_input_router.route_frame(input);
    dispatch_gameplay_frame(gameplay_frame);

    for (const elysia::input::RawInputEvent& raw_event : events)
    {
        dispatch_gameplay_events(_gameplay_input_router.route_event(raw_event));
    }
}

bool ApplicationScene::accepts_gameplay_input() const
{
    return false;
}

void ApplicationScene::on_scene_object_registered(elysia::core::SceneObject& object)
{
    elysia::scene::Scene::on_scene_object_registered(object);

    if (auto* receiver = dynamic_cast<arcneco::input::GameplayInputFrameReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _gameplay_frame_receivers,
            GameplayInputFrameReceiverEntry{ &object, receiver }
        );
    }

    if (auto* receiver = dynamic_cast<arcneco::input::GameplayInputEventReceiver*>(&object))
    {
        elysia::scene::scene_input_order::insert_receiver_entry_sorted(
            _gameplay_event_receivers,
            GameplayInputEventReceiverEntry{ &object, receiver }
        );
    }
}

void ApplicationScene::prune_domain_receivers()
{
    erase_destroyed_entries(_gameplay_frame_receivers);
    erase_destroyed_entries(_gameplay_event_receivers);
}

void ApplicationScene::dispatch_gameplay_frame(const arcneco::input::GameplayInputFrame& input)
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

void ApplicationScene::dispatch_gameplay_events(const std::vector<arcneco::input::GameplayInputEvent>& events)
{
    for (const arcneco::input::GameplayInputEvent& gameplay_event : events)
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
