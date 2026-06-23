#pragma once

#include "../../engine/scene/scene.h"
#include "../input/gameplay_input_router.h"

#include "../../gameplay/input/contracts/gameplay_input_event_receiver.h"
#include "../../gameplay/input/contracts/gameplay_input_frame_receiver.h"

#include <vector>

class ApplicationScene : public elysia::scene::Scene
{
public:
    void on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events) override;

protected:
    [[nodiscard]] virtual bool accepts_gameplay_input() const;
    void on_scene_object_registered(elysia::core::SceneObject& object) override;

private:
    struct GameplayInputFrameReceiverEntry
    {
        elysia::core::SceneObject* object = nullptr;
        arcneco::input::GameplayInputFrameReceiver* receiver = nullptr;
    };

    struct GameplayInputEventReceiverEntry
    {
        elysia::core::SceneObject* object = nullptr;
        arcneco::input::GameplayInputEventReceiver* receiver = nullptr;
    };

private:
    void prune_domain_receivers();
    void dispatch_gameplay_frame(const arcneco::input::GameplayInputFrame& input);
    void dispatch_gameplay_events(const std::vector<arcneco::input::GameplayInputEvent>& events);

private:
    GameplayInputRouter _gameplay_input_router;
    std::vector<GameplayInputFrameReceiverEntry> _gameplay_frame_receivers;
    std::vector<GameplayInputEventReceiverEntry> _gameplay_event_receivers;
};
